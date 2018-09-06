#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "washroom.h"
#include "joystick.h"
#include "stall.h"
#include "file.h"
#include "network.h"

static int setWashroomId(void);
static void* threadActivity(void* arg);
static void initializePassword();
static void setDigit(int leftDigit, int rightDigit);
static void displayDigit(int digit, char *filepath);
static void turnOff(void);
static void setSingleDigit(int digit);
static void delay5MSecs(void);
static void initializeFiles(void);
static int getI2cFd(void);
static void setToOutput(void);
static void writeToRegister(int fd, unsigned char registerAddress, unsigned char value);
static void setStall(int *stallCount, int value);

#define SLOTS_FILEPATH "/sys/devices/platform/bone_capemgr/slots"
#define EXPORT_FILEPATH "/sys/class/gpio/export"
#define LEFT_DIRECTION_FILEPATH "/sys/class/gpio/gpio61/direction"
#define RIGHT_DIRECTION_FILEPATH "/sys/class/gpio/gpio44/direction"
#define LEFT_VALUE_FILEPATH "/sys/class/gpio/gpio61/value"
#define RIGHT_VALUE_FILEPATH "/sys/class/gpio/gpio44/value"
#define ENABLE_I2C "BB-I2C1"
#define DIRECTION_OUT "out"
#define LEFT_DIGIT "61"
#define RIGHT_DIGIT "44"
#define I2C_BUS1 "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define OUTPUT_VALUE 0x00
#define POWER_ON "1"
#define POWER_OFF "0"
#define REG_OUTA 0x14
#define REG_OUTB 0x15
#define REGISTER_BUFFER_SIZE 2
#define DIGIT_COUNT 10

static const int REGISTER_A[DIGIT_COUNT] = {0xa1, 0x80, 0x31, 0xb0, 0x90, 0xb0, 0xb1, 0x80, 0xb1, 0xb0};
static const int REGISTER_B[DIGIT_COUNT] = {0x86, 0x02, 0x0f, 0x0f, 0x8a, 0x8c, 0x8c, 0x06, 0x8f, 0x8f};
static pthread_t threadId;
static int fd;
static bool keepGoing;
static int numOpenStalls = 0; //TODO: make thread safe
static int totalStalls = 0;
static int washroomId;
static const int ID_PRIMARY = 0;
static const int ID_SECONDARY = 1;
static const int ID_ERROR = 2;
static const int MAX_STALL_COUNT = 9;
static const int MIN_STALL_COUNT = 0;
static int adminPass;

void Washroom_start() {
	//Set up zen cape display and thread
	keepGoing = true;
	initializeFiles();
	washroomId = setWashroomId();
	// washroomId = ID_PRIMARY;
	Network_portInit(washroomId);

	fd = getI2cFd();
	setToOutput();
	if(washroomId==0){
		initializePassword();
	}

	pthread_create(&threadId, NULL, &threadActivity, NULL);
}

// For two beaglebones, set id by joystick //
static int setWashroomId() {
	printf("Press UP to set as primary washroom, DOWN to set as secondary.\n");
	jsInput_t input = Joystick_getInput();
	if (input == JS_UP) {
		return ID_PRIMARY;
	} else if (input == JS_DOWN) {
		return ID_SECONDARY;
	}
	return ID_ERROR;
}

//creates a unique password at startup for admin use on webapp
static void initializePassword(){
	//seed the random function
	srand(time(NULL));   // should only be called once
	//set the password to a 4 digit number
	adminPass = rand()%9999;
	printf("Admin Password: %04d\n", adminPass);
}

_Bool Washroom_verifyPassword(int password){
	return (password==adminPass);
}

static void initializeFiles() {
	File_writeStringToFile(SLOTS_FILEPATH, ENABLE_I2C);
	File_writeStringToFile(EXPORT_FILEPATH, LEFT_DIGIT);
	File_writeStringToFile(EXPORT_FILEPATH, RIGHT_DIGIT);
	File_writeStringToFile(LEFT_DIRECTION_FILEPATH, DIRECTION_OUT);
	File_writeStringToFile(RIGHT_DIRECTION_FILEPATH, DIRECTION_OUT);
}

static int getI2cFd() {
	int fd = open(I2C_BUS1, O_RDWR);
	if (fd < 0) {
		printf("Error opening %s\n", I2C_BUS1);
	}
	int result = ioctl(fd, I2C_SLAVE, I2C_DEVICE_ADDRESS);
	if (result < 0) {
		printf("Error opening setting device to slave address");
	}
	return fd;
}

static void setToOutput() {
	writeToRegister(fd, REG_DIRA, OUTPUT_VALUE);
	writeToRegister(fd, REG_DIRB, OUTPUT_VALUE);
}

static void writeToRegister(int fd, unsigned char registerAddress, unsigned char value) {
	if(keepGoing){
		unsigned char buffer[REGISTER_BUFFER_SIZE];
		buffer[0] = registerAddress;
		buffer[1] = value;
		int result = write(fd, buffer, REGISTER_BUFFER_SIZE);
		if (result != REGISTER_BUFFER_SIZE) {
			printf("Error writing to i2c register\n");
		}
	}
}

void Washroom_setTotalStalls(int value) {
	setStall(&totalStalls, value);
}

void Washroom_setNumOpenStalls(int value) {
	setStall(&numOpenStalls, value);

}

int Washroom_getTotalStallCount(void) {
	return(totalStalls);

}

int Washroom_getOpenStallCount(void) {
	return(numOpenStalls);

}

int Washroom_reserveStall(void) {
	int full = 0;

	if(numOpenStalls == full) {
		return full;
	}

	return Stall_reserveStall();
}

void Washroom_setStallMaintenance(int num) {
	Stall_setStallMaintenance(num);
}

void Washroom_releaseStallMaintenance(int num){
	Stall_releaseStallMaintenance(num);
}
//Pass in an array of size Washroom_getTotalStallCount()
//Postcondition: array is filled with stall states
void Washroom_getStallStates(int* array) {
	Stall_getStallStates(array);
}

static void setStall(int *stallCount, int value) {
	if(value > MAX_STALL_COUNT) {
		value = MAX_STALL_COUNT;
	} else if (value < MIN_STALL_COUNT) {
		value = MIN_STALL_COUNT;
	}
	*stallCount = value;
}

static void *threadActivity(void *arg) {
	printf("washroom id: %d\n", washroomId);
	while(keepGoing) {
		setDigit(numOpenStalls, totalStalls);
	}
	return NULL;
}

static void setDigit(int leftDigit, int rightDigit) {
	displayDigit(leftDigit, LEFT_VALUE_FILEPATH);
	displayDigit(rightDigit, RIGHT_VALUE_FILEPATH);
}

static void displayDigit(int digit, char *filepath) {
	turnOff();
	setSingleDigit(digit);
	File_writeStringToFile(filepath, POWER_ON);
	delay5MSecs();
}

static void turnOff() {
	File_writeStringToFile(LEFT_VALUE_FILEPATH, POWER_OFF);
	File_writeStringToFile(RIGHT_VALUE_FILEPATH, POWER_OFF);
}

static void setSingleDigit(int digit) {
	writeToRegister(fd, REG_OUTA, REGISTER_A[digit]);
	writeToRegister(fd, REG_OUTB, REGISTER_B[digit]);
}

static void delay5MSecs() {
	long seconds = 0;
	long nanoseconds = 5000000;
	struct timespec delay = {seconds, nanoseconds};
	nanosleep(&delay, NULL);
}

void Washroom_stop() {
	keepGoing = false;
	turnOff();
	close(fd);
	pthread_join(threadId, NULL);
}

