#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include "joystick.h"
#include "file.h"

#define EXPORT_FILEPATH "/sys/class/gpio/export"
#define GPIO_FILEPATH "/sys/class/gpio/gpio%d"
#define DIRECTION_SUFFIX "/direction"
#define EDGE_SUFFIX "/edge"
#define VALUE_SUFFIX "/value"
#define GPIO_LENGTH 4
#define BUFFER_SIZE 64

//Private function declarations

static void initializeGPIOs(void);
static void initializeFilepaths(void);
static void insertGpioNumberInFilepath(char *suffix, char filepath[][BUFFER_SIZE]);
static void configureGpioSettings(void);
static void initializePoll(void);
static void clearPollBuffer(void);
static void setEdges(const char *setting);
static void resetRevents(void);
static void waitForJoystickRelease(void);
static int getPollFileDescriptor(char *filepath);

//Private variables

typedef struct GPIO {
	int number;
	jsInput_t direction;
	char *directionFilepath;
	char *edgeFilepath;
	char *valueFilepath;
} GPIO;

static const int gpioNumbers[GPIO_LENGTH] = {26, 46, 65, 47};
static const jsInput_t gpioJsInputs[GPIO_LENGTH] = {JS_UP, JS_DOWN, JS_LEFT, JS_RIGHT};
static char gpioDirFilepaths[GPIO_LENGTH][BUFFER_SIZE];
static char gpioEdgeFilepaths[GPIO_LENGTH][BUFFER_SIZE];
static char gpioValueFilepaths[GPIO_LENGTH][BUFFER_SIZE];
static GPIO gpios[GPIO_LENGTH];

const static char *RECIEVE_INPUT = "in";
const static char *DETECT_RELEASE = "rising";
const static char *DETECT_PRESS = "falling";

static struct pollfd fileDescriptors[GPIO_LENGTH];
static const int TIMEOUT_MSECS = 1000 * 60 * 5;
static int catchEvent;
static char pollBuffer[BUFFER_SIZE];

//Functions

void Joystick_start() {
	initializeGPIOs();
	configureGpioSettings();
	initializePoll();
}

static void initializeGPIOs() {
	initializeFilepaths();
	for (int i = 0; i < GPIO_LENGTH; i++) {
		gpios[i] = (GPIO) {gpioNumbers[i], gpioJsInputs[i], gpioDirFilepaths[i], gpioEdgeFilepaths[i], gpioValueFilepaths[i]};
	}
}

static void initializeFilepaths() {
	insertGpioNumberInFilepath(DIRECTION_SUFFIX, gpioDirFilepaths);
	insertGpioNumberInFilepath(EDGE_SUFFIX, gpioEdgeFilepaths);
	insertGpioNumberInFilepath(VALUE_SUFFIX, gpioValueFilepaths);
}

static void insertGpioNumberInFilepath(char *suffix, char filepaths[][BUFFER_SIZE]) {
	for(int i = 0; i < GPIO_LENGTH; i++) {
		char buffer[BUFFER_SIZE];
		memset(filepaths[i], '\0', sizeof(char) * BUFFER_SIZE);
		memset(buffer, '\0', sizeof(char) * BUFFER_SIZE);

		strcat(buffer, GPIO_FILEPATH);
		strcat(buffer, suffix);
		sprintf(buffer, buffer, gpioNumbers[i]);
		strcpy(filepaths[i], buffer);
	}
}

static void configureGpioSettings() {
	for (int i = 0; i < GPIO_LENGTH; i++) {
		File_writeIntToFile(EXPORT_FILEPATH, gpios[i].number);
		File_writeStringToFile(gpios[i].directionFilepath, RECIEVE_INPUT);
	}
}

static void initializePoll() {
	for(int i = 0; i < GPIO_LENGTH; i++) {
		fileDescriptors[i].fd = getPollFileDescriptor(gpios[i].valueFilepath);
		fileDescriptors[i].events = POLLPRI;
	}
	clearPollBuffer();
}

static void clearPollBuffer() {
	const int NO_TIME = 0; 
	poll(fileDescriptors, GPIO_LENGTH, NO_TIME);
	for(int i = 0; i < GPIO_LENGTH; i++) {
		read(fileDescriptors[i].fd, pollBuffer, BUFFER_SIZE);
	}
}

jsInput_t Joystick_getInput() {
	resetRevents();
	setEdges(DETECT_PRESS);
	catchEvent = poll(fileDescriptors, GPIO_LENGTH, TIMEOUT_MSECS);
	for (int i = 0; i < GPIO_LENGTH; i++) {
		read(fileDescriptors[i].fd, pollBuffer, BUFFER_SIZE);
		if(catchEvent > 0) {
			if(fileDescriptors[i].revents & POLLPRI) {
				waitForJoystickRelease();
				return gpios[i].direction;
			}
		}
	}
	return JS_NONE;
}

static void waitForJoystickRelease() {
	resetRevents();
	setEdges(DETECT_RELEASE);
	catchEvent = poll(fileDescriptors, GPIO_LENGTH, TIMEOUT_MSECS);
	for (int i = 0; i < GPIO_LENGTH; i++) {
		read(fileDescriptors[i].fd, pollBuffer, BUFFER_SIZE);
	}
}

static void setEdges(const char *setting) {
	for (int i = 0; i < GPIO_LENGTH; i++) {
		File_writeStringToFile(gpios[i].edgeFilepath, setting);
	}
}

static void resetRevents() {
	int startIndex = 0;
	for (int i = 0; i < GPIO_LENGTH; i++) {
		fileDescriptors[i].revents = 0;
		lseek(fileDescriptors[i].fd, startIndex, SEEK_SET);
	}
}

static int getPollFileDescriptor(char *filepath) {
	int fd;
	fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		printf("getPollFileDescriptor: Error opening %s\n", filepath);
	}
	return fd;
}

void Joystick_stop() {
	for(int i = 0; i < GPIO_LENGTH; i++) {
		close(fileDescriptors[i].fd);
	}
}