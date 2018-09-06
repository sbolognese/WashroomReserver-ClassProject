/*
 * Joystick handles waiting for and determining joystick inputs
 */
#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef enum JoystickInput {
	JS_UP,
	JS_DOWN,
	JS_LEFT,
	JS_RIGHT,
	JS_NONE
} jsInput_t;

void Joystick_start(void);
jsInput_t Joystick_getInput(void);
void Joystick_stop(void);

#endif