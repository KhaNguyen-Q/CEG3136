/*
 * game.c
 *
 *  Created on: Oct 20, 2025
 *      Author: bguer053
 */


#include "game.h"
#include "gpio.h"
#include "systick.h"
#include "display.h"

#define SHIFT_TIME 200

static Time_t timeShift; // Time LED position was last shifted
static int position = 0; // Current position of illuminated LED
static int direction = 0; // Current LED shift direction: 0=right 1=left
static int reversed = 0; // Flag to indicate direction has been reversed

void Init_Game (void) {
 GPIO_PortEnable(GPIOX);
 timeShift = TimeNow();

 DisplayEnable();
 DisplayColor(WHITE);

 DisplayPrint(0, "Linear pong");
 DisplayPrint(1, "press a button");
}
void Task_Game (void) {
 if (TimePassed(timeShift) >= SHIFT_TIME) {
 // Move LED position by 1, depending on direction, wrapping at end
 if (position == (direction ? 7 : 0))
 position = direction ? 0 : 7;
 else
 position += direction ? +1 : -1;
 GPIO_PortOutput(GPIOX, 1 << position); // Update LEDs
 timeShift = TimeNow();
 reversed = 0; // Check for button presses once again
 }
 // Reverse direction if button next to illuminated LED is pressed
 if (!reversed && GPIO_PortInput(GPIOX) & 1 << (position + 8)) {
 direction ^= 1;
 reversed = 1; // Ignore button until LED moves to next position
 }

 UpdateDisplay();
}
