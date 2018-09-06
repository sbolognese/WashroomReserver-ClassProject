/*
 * 
 */
#ifndef LED_H
#define LED_H

typedef enum COLOUR {
	RED = 0,
	GREEN = 1,
	BLUE = 2
} COLOUR;

void Led_start(void);
void Led_setColour(int id, COLOUR colour);

#endif