#include <unistd.h> //sleep
#include <pthread.h>
#include <stdio.h>
#include "main.h"
#include "washroom.h"
#include "stall.h"
#include "network.h"
#include "joystick.h"
#include "led.h"

static void start(void);
static void stop(void);

static pthread_barrier_t mainBarrier;
static const int BARRIER_COUNT = 2;


int main() {
	start();
	// Washroom_reserveStall();
	// sleep(15); // Delete me (temporary sleep 15 seconds then shut down)
	pthread_barrier_wait(&mainBarrier); //Use instead of sleep to wait until barrier broken
	stop();
}

static void start() {
	pthread_barrier_init(&mainBarrier, NULL, BARRIER_COUNT);
	Led_start(); //Before stall
	Stall_start(); // Before Washroom
	Joystick_start(); // Before Washroom
	Washroom_start(); //Before Network
	
	Network_start();
}

static void stop() {
	Washroom_stop();
	Stall_stop();
	Joystick_stop();
	Network_stop();
}

//Call to break barrier and turn off app
void Main_stopProgram() {
	pthread_barrier_destroy(&mainBarrier);
	pthread_barrier_wait(&mainBarrier);
}
