#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h> //sleep
#include "washroom.h"
#include "stall.h"
#include "file.h"
#include "led.h"

#define FILEPATH_BUFFER_SIZE 250
#define POLL_BUFFER_SIZE 64
#define EXPORT_FILEPATH "/sys/class/gpio/export"
#define EDGE_FILEPATH "/sys/class/gpio/gpio%d/edge"
#define DIRECTION_FILEPATH "/sys/class/gpio/gpio%d/direction"
#define VALUE_FILEPATH "/sys/class/gpio/gpio%d/value"
#define SET_TO_READ "in"
#define DETECT_OPEN_AND_CLOSE "both"

// When adding stall, only need to add GPIO to gpioNumbers and increment TOTAL_STALLS
#define TOTAL_STALLS 2
// GPIO numbers for each stall - Stall 1 GPIO=50 for P9.14
//						 	   - Stall 2 GPIO=51 for P9.16
static const int gpioNumbers[TOTAL_STALLS] = {50, 51};
// static const int gpioNumbers[TOTAL_STALLS] = {51};

typedef struct Stall {
	int id;
	int status;
	int gpioNumber;
	char directionFilepath[FILEPATH_BUFFER_SIZE];
	char edgeFilepath[FILEPATH_BUFFER_SIZE];
	char valueFilepath[FILEPATH_BUFFER_SIZE];
} stall_t;

static stall_t stalls[TOTAL_STALLS];
static pthread_t threadId;
static bool keepGoing;
static const int STALL_OPEN = 0;
static const int STALL_CLOSE = 1;
static const int STALL_MAINT = 2;
static const int STALL_RESERVE = 3;

static struct pollfd fds[TOTAL_STALLS];
static int catchEvent;
static char pollBuffer[POLL_BUFFER_SIZE];
static const int POLL_TIME_MSECS = 1000 * 60 * 10;

static void initializeStalls(void);
static void setStallGpioFilepaths(stall_t *stall, int gpioNumber);
static void configureGpio(stall_t *stall);
static void setStallStatus(stall_t *stall);
static void initializePoll(void);
static void clearPollBuffer(void);
static int getPollFD(char *filepath);
static void resetRevents(void);
static void* threadActivity(void* arg);
static void update(void);
static void updateNumOpenStalls(void);
static void updateStallLed(void);

void Stall_start(void) {
	Washroom_setTotalStalls(TOTAL_STALLS);
	initializeStalls();
	initializePoll();
	keepGoing = true;
	pthread_create(&threadId, NULL, &threadActivity, NULL);
}

// Set each stall a unique id, default to stall is open,
// and set up GPIO filepaths and configure GPIO settings
static void initializeStalls() {
	for (int i = 0; i < TOTAL_STALLS; i++) {
		stall_t *stall = &stalls[i];
		stall->id = i;
		setStallGpioFilepaths(stall, gpioNumbers[i]);
		configureGpio(stall);
		setStallStatus(stall);
	}
	update();
}

// Insert GPIO number into filepaths
static void setStallGpioFilepaths(stall_t *stall, int gpioNumber) {
	stall->gpioNumber = gpioNumber;
	sprintf(stall->directionFilepath, DIRECTION_FILEPATH, gpioNumber);
	sprintf(stall->edgeFilepath, EDGE_FILEPATH, gpioNumber);
	sprintf(stall->valueFilepath, VALUE_FILEPATH, gpioNumber);
}

// Export stall's pin and set to read
static void configureGpio(stall_t *stall) {
	File_writeIntToFile(EXPORT_FILEPATH, stall->gpioNumber);
	File_writeStringToFile(stall->directionFilepath, SET_TO_READ);
	File_writeStringToFile(stall->edgeFilepath, DETECT_OPEN_AND_CLOSE);
}

static void setStallStatus(stall_t *stall) {
	FILE *file = fopen(stall->valueFilepath, "r");
	if (file == NULL) {
		printf("Error opening %s", stall->valueFilepath);
	}
	int itemsRead = fscanf(file, "%s", pollBuffer);
	if (itemsRead <= 0) {
		printf("Error reading %s", stall->valueFilepath);
	}
	fclose(file);
	sscanf(pollBuffer, "%d", &(stall->status));
}

static void initializePoll() {
	for (int i = 0; i < TOTAL_STALLS; i++) {
		fds[i].fd = getPollFD(stalls[i].valueFilepath);
		fds[i].events = POLLPRI;
	}
	clearPollBuffer();
}

static int getPollFD(char *filepath) {
	int fd;
	fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		printf("Stall: Error opening %s\n", filepath);
	}
	return fd;
}

static void clearPollBuffer() {
	const int NO_TIME = 0; 
	poll(fds, TOTAL_STALLS, NO_TIME);
	for(int i = 0; i < TOTAL_STALLS; i++) {
		read(fds[i].fd, pollBuffer, POLL_BUFFER_SIZE);
	}
}

static void* threadActivity(void* arg) {
	while(keepGoing) {
		resetRevents();
		catchEvent = poll(fds, TOTAL_STALLS, POLL_TIME_MSECS);
		for(int i = 0; i < TOTAL_STALLS; i++) {
			read(fds[i].fd, pollBuffer, POLL_BUFFER_SIZE);
			if (catchEvent > 0) {
				if (fds[i].revents & POLLPRI) {
					sscanf(pollBuffer, "%d", &(stalls[i].status));
					update();
				}
			}
		}
	}
	return NULL;
}

static void resetRevents() {
	int startIndex = 0;
	for (int i = 0; i < TOTAL_STALLS; i++) {
		fds[i].revents = 0;
		lseek(fds[i].fd, startIndex, SEEK_SET);
	}
}

void Stall_stop(void) {
	for(int i = 0; i < TOTAL_STALLS; i++) {
		close(fds[i].fd);
	}
	keepGoing = false;
	pthread_join(threadId, NULL);
}

static void update() {
	updateNumOpenStalls();
	updateStallLed();
}

static void updateNumOpenStalls(void) {
	int numOpenStalls = 0;
	for (int i = 0; i < TOTAL_STALLS; i++) {
		if(stalls[i].status == STALL_OPEN) {
			numOpenStalls++;
		}
	}
	Washroom_setNumOpenStalls(numOpenStalls);
}

static void updateStallLed() {
	for (int i = 0; i < TOTAL_STALLS; i++) {
		if(stalls[i].status == STALL_OPEN) {
			Led_setColour(i, GREEN);
		} else if(stalls[i].status == STALL_CLOSE || stalls[i].status == STALL_MAINT) {
			Led_setColour(i, RED);
		} else if(stalls[i].status == STALL_RESERVE) {
			Led_setColour(i, BLUE);
		}
	}
}

int Stall_reserveStall() {
	int success = 1;
	int full = 0;

	for(int i = 0; i < TOTAL_STALLS; i++) {
		if(stalls[i].status == STALL_OPEN) {
			stalls[i].status = STALL_RESERVE;
			update();
			return success;
		}
	}
	return full;
}

void Stall_setStallMaintenance(int stallNum){
	stalls[stallNum].status = STALL_MAINT;
	update();
}

void Stall_releaseStallMaintenance(int stallNum){
	stalls[stallNum].status = STALL_OPEN;
	update();
}

void Stall_getStallStates(int* array) {
	for(int i = 0; i < TOTAL_STALLS; i++) {
		array[i] = stalls[i].status;
	}
}