/*
 * alarm.c
 *
 *  Created on: Sep 29, 2025
 *      Author: bguer053
 */


// Alarm system app
#include <stdio.h>
#include <stdbool.h>
#include "alarm.h"
#include "systick.h"
#include "gpio.h"
#include "display.h"

// Macros for LED control using GPIO driver
#define LED_Set(pin)    GPIO_Output(pin, HIGH)
#define LED_Clear(pin)  GPIO_Output(pin, LOW)
#define LED_Toggle(pin) GPIO_Toggle(pin)

// GPIO pins
static const Pin_t LedR = {GPIOB, 9};
static const Pin_t LedG = {GPIOC, 7};
static const Pin_t LedB = {GPIOB, 7};
static const Pin_t Button = {GPIOC, 13};
static const Pin_t Motion = {GPIOB, 8};

// Alarm states
static enum { DISARMED, ARMED, TRIGGERED } state;

// Constants
#define DEBOUNCE_MS        50u
#define BRIEF_PRESS_MAX_MS 2000u
#define LONG_PRESS_MS      3000u
#define LED_TOGGLE_MS      1000u
#define TASK_TICK_MS       1u

// Variables
static bool motionFlag = false;
static bool buttonPressedFlag = false;
static bool buttonBriefFlag = false;
static uint32_t lastButtonPressTime = 0;
static uint32_t lastButtonReleaseTime = 0;
static uint32_t armedSince = 0;
static uint32_t ledToggleSince = 0;
static bool ledStateGB = false;

// Callback prototypes
static void CallbackMotionDetect(void);
static void CallbackButtonPress(void);
static void CallbackButtonRelease(void);

// Initialization
void Init_Alarm(void) {
    state = DISARMED;

    motionFlag = false;
    buttonPressedFlag = false;
    buttonBriefFlag = false;
    lastButtonPressTime = 0;
    lastButtonReleaseTime = 0;
    armedSince = 0;

    GPIO_Enable(LedR); GPIO_Mode(LedR, OUTPUT);
    GPIO_Enable(LedG); GPIO_Mode(LedG, OUTPUT);
    GPIO_Enable(LedB); GPIO_Mode(LedB, OUTPUT);

    LED_Clear(LedR);
    LED_Clear(LedG);
    LED_Clear(LedB);

    GPIO_Enable(Motion);
    GPIO_Mode(Motion, INPUT);

    GPIO_Enable(Button);
    GPIO_Mode(Button, INPUT);

    GPIO_Callback(Motion, CallbackMotionDetect, RISE);
    GPIO_Callback(Button, CallbackButtonPress, RISE);
    GPIO_Callback(Button, CallbackButtonRelease, FALL);

    DisplayEnable();
    DisplayColor(WHITE);
    DisplayPrint(0, "DISARMED");
}

// Task (state machine)
void Task_Alarm(void) {
    uint32_t now = TimeNow();

    switch (state) {
    case DISARMED:
        LED_Clear(LedR);
        LED_Clear(LedG);
        LED_Clear(LedB);

        if (buttonBriefFlag) {
            buttonBriefFlag = 0;
            state = ARMED;
            armedSince = now;
            ledToggleSince = now;
            ledStateGB = false;
            LED_Set(LedG);
            LED_Clear(LedB);
            DisplayColor(WHITE);
            DisplayPrint(0, "DISARMED");
        }
        break;

    case ARMED:
        if (buttonPressedFlag) {
            uint32_t held = now - lastButtonPressTime;
            if (held >= LONG_PRESS_MS) {
                buttonPressedFlag = 0;
                buttonBriefFlag = 0;
                state = DISARMED;
                DisplayColor(YELLOW);
                DisplayPrint(0, "ARMED");

            }
        }

        if ((now - ledToggleSince) >= LED_TOGGLE_MS) {
            ledToggleSince = now;
            ledStateGB = !ledStateGB;
            if (ledStateGB) {
                LED_Clear(LedG);
                LED_Set(LedB);
            } else {
                LED_Set(LedG);
                LED_Clear(LedB);
            }
        }

        if (motionFlag) {
            motionFlag = 0;
            state = TRIGGERED;
            LED_Set(LedR);
            LED_Clear(LedG);
            LED_Clear(LedB);

        }
        break;

    case TRIGGERED:
        LED_Set(LedR);
        LED_Clear(LedG);
        LED_Clear(LedB);

        if (buttonBriefFlag) {
            buttonBriefFlag = 0;
            state = ARMED;
            armedSince = now;
            ledToggleSince = now;
            ledStateGB = false;
            LED_Set(LedG);
            LED_Clear(LedR);
            DisplayColor(RED);
            DisplayPrint(0, "TRIGGERED");
        }

        if (buttonPressedFlag) {
            uint32_t held = now - lastButtonPressTime;
            if (held >= LONG_PRESS_MS) {
                buttonPressedFlag = 0;
                buttonBriefFlag = 0;
                state = DISARMED;
                LED_Set(LedR);
                LED_Clear(LedG);
                LED_Clear(LedB);
            }
        }
        break;

    default:
        state = DISARMED;
        LED_Set(LedR);
        LED_Clear(LedG);
        LED_Clear(LedB);
        break;
    }
}

// ------------------------------------------------------------
// Interrupt callback functions
// ------------------------------------------------------------
void CallbackMotionDetect(void) {
    motionFlag = true;
}

void CallbackButtonPress(void) {
    lastButtonPressTime = TimeNow();
    buttonPressedFlag = true;
}

void CallbackButtonRelease(void) {
    uint32_t now = TimeNow();
    if ((now - lastButtonPressTime) < BRIEF_PRESS_MAX_MS)
        buttonBriefFlag = true;
    buttonPressedFlag = false;
}



