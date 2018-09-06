#include <stdio.h>
#include <string.h>
#include "led.h"
#include "file.h"
#include "stall.h"

#define EXPORT_FILEPATH "/sys/class/gpio/export"
#define GPIO_FILEPATH "/sys/class/gpio/gpio%d"
#define DIRECTION_SUFFIX "/direction"
#define VALUE_SUFFIX "/value"
#define GPIO_LENGTH 3
#define BUFFER_SIZE 64
#define LED_DIR_OUT "out"
#define TOTAL_LEDS 2

static void initializeGPIOs(void);
static void initializeFilepaths(void);
static void insertGpioNumberInFilepath(char *suffix, char filepath[][GPIO_LENGTH][BUFFER_SIZE]);
static void configureGpioSettings(void);
static void turnOffAllColours(int id);

typedef struct LED_GPIO {
	int number;
	COLOUR colour;
	char *directionFilepath;
	char *valueFilepath;
} LED_GPIO;
 
static const int gpioNumbers[TOTAL_LEDS][GPIO_LENGTH] = {{30, 60, 31}, //Stall1: {R=P9_11, G=P9_12, B=P9_13}
														 {3, 2, 49}}; //Stall2: {R=P9_21, G=P9_22, B=P9_23}
static const COLOUR gpioColours[GPIO_LENGTH] = {RED, GREEN, BLUE};
static char gpioDirFilepaths[TOTAL_LEDS][GPIO_LENGTH][BUFFER_SIZE];
static char gpioValueFilepaths[TOTAL_LEDS][GPIO_LENGTH][BUFFER_SIZE];
static LED_GPIO gpios[TOTAL_LEDS][GPIO_LENGTH];

static const int TURN_OFF = 1;
static const int TURN_ON = 0;


void Led_start() {
	initializeGPIOs();
	configureGpioSettings();
}

static void initializeGPIOs() {
	initializeFilepaths();
	for (int h = 0; h < TOTAL_LEDS; h++) {
		for (int i = 0; i < GPIO_LENGTH; i++) {
			gpios[h][i] = (LED_GPIO) {gpioNumbers[h][i], gpioColours[i], gpioDirFilepaths[h][i], gpioValueFilepaths[h][i]};
		}
	}
}

static void initializeFilepaths() {
	insertGpioNumberInFilepath(DIRECTION_SUFFIX, gpioDirFilepaths);
	insertGpioNumberInFilepath(VALUE_SUFFIX, gpioValueFilepaths);
}

static void insertGpioNumberInFilepath(char *suffix, char filepaths[][GPIO_LENGTH][BUFFER_SIZE]) {
	for(int h = 0; h < TOTAL_LEDS; h++) {
		for(int i = 0; i < GPIO_LENGTH; i++) {
			char buffer[BUFFER_SIZE];
			memset(filepaths[h][i], '\0', sizeof(char) * BUFFER_SIZE);
			memset(buffer, '\0', sizeof(char) * BUFFER_SIZE);

			strcat(buffer, GPIO_FILEPATH);
			strcat(buffer, suffix);
			sprintf(buffer, buffer, gpioNumbers[h][i]);
			strcpy(filepaths[h][i], buffer);
		}
	}
}

static void configureGpioSettings() {
	for(int h = 0; h < TOTAL_LEDS; h++) {
		for (int i = 0; i < GPIO_LENGTH; i++) {
			File_writeIntToFile(EXPORT_FILEPATH, gpios[h][i].number);
			File_writeStringToFile(gpios[h][i].directionFilepath, LED_DIR_OUT);
		}
	}
}

void Led_setColour(int id, COLOUR colour) {
	turnOffAllColours(id);
	File_writeIntToFile(gpios[id][colour].valueFilepath, TURN_ON);
}


static void turnOffAllColours(int id) {
	for (int i = 0; i < GPIO_LENGTH; i++) {
		File_writeIntToFile(gpios[id][i].valueFilepath, TURN_OFF);
	}
}