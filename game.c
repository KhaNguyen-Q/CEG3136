/*
 * game.c
 *
 *  Created on: Nov 1, 2025
 *      Author: bguer053
 */

#include <stdint.h>
#include <stdbool.h>
#include "game.h"
#include "gpio.h"
#include "systick.h"
#include "display.h"
#include <stdio.h>

// --------------------------------------------------------
// Constants
// --------------------------------------------------------
#define SPEED_SLOW 150U
#define SPEED_MED 110U
#define SPEED_FAST 70U
#define NUM_SPEEDS 3
#define QUIT_HOLD_MS 3000U

// Button bit mapping
#define BTN_P1_MASK ((1<<8)|(1<<9)|(1<<10))
#define BTN_P2_MASK ((1<<13)|(1<<14)|(1<<15))
#define BTN_START_BIT (1<<11)
#define BTN_SELECT_BIT (1<<12)


#define LEFT_EDGE 0
#define RIGHT_EDGE 7

// --------------------------------------------------------
// Game state machine
// --------------------------------------------------------
static enum { TITLE, SERVE, PLAY, WIN } state;

// --------------------------------------------------------
// Module-scope variables
// --------------------------------------------------------
static Time_t timeShift;
static int position = 0;
static int direction = 0; // 0 = right, 1 = left
static int speedIndex = 0; // 0 slow, 1 med, 2 fast
static uint32_t speedTable[NUM_SPEEDS] = { SPEED_SLOW, SPEED_MED, SPEED_FAST };

static int P1score = 0;
static int P2score = 0;
static bool firstServe = true;
static bool P1serve = true;

static Time_t startHoldTime = 0;
static bool startHeld = false;

// --------------------------------------------------------
// Initialization
// --------------------------------------------------------
void Init_Game(void) {
GPIO_PortEnable(GPIOX);
DisplayEnable();
DisplayColor(ALARM, WHITE);

state = TITLE;
P1score = 0;
P2score = 0;
firstServe = true;
P1serve = true;
speedIndex = 0;
position = 0;
direction = 0;

GPIO_PortOutput(GPIOX, 1 << position);
DisplayPrint(ALARM, 0, "Linear Pong");
DisplayPrint(ALARM, 1, "Press Start");

timeShift = TimeNow();


}

// --------------------------------------------------------
// Periodic Task
// --------------------------------------------------------
void Task_Game(void) {
uint32_t input = GPIO_PortInput(GPIOX);

// ---------------- Global Quit Detection ----------------
if ((input & BTN_START_BIT) && !startHeld) {
    startHeld = true;
    startHoldTime = TimeNow();
    GPIO_PortOutput(GPIOX, 0x00);
} else if (!(input & BTN_START_BIT)) {
    startHeld = false;
}

if (state != TITLE && startHeld && TimePassed(startHoldTime) >= QUIT_HOLD_MS) {
    while (GPIO_PortInput(GPIOX) & BTN_START_BIT); // wait for release
    // Reset
    P1score = P2score = 0;
    firstServe = true;
    P1serve = true;
    speedIndex = 0;
    position = 0;
    direction = 0;
    DisplayColor(ALARM, WHITE);
    DisplayPrint(ALARM, 0, "Linear Pong");
    DisplayPrint(ALARM, 1, "Press Start");
    GPIO_PortOutput(GPIOX, 1 << position);
    timeShift = TimeNow();
    state = TITLE;
    return;
}

switch (state) {
// --------------------------------------------------------
// TITLE: Ball bounces, Select adjusts speed, Start begins
// --------------------------------------------------------
case TITLE: {
    if (TimePassed(timeShift) >= speedTable[speedIndex]) {
        if (position == RIGHT_EDGE) direction = 1;
        else if (position == LEFT_EDGE) direction = 0;
        position += direction ? -1 : +1;
        GPIO_PortOutput(GPIOX, 1 << position);
        timeShift = TimeNow();
    }

    static bool prevSelect = false;
    bool currSelect = (input & BTN_SELECT_BIT);
    if (currSelect && !prevSelect) {
        speedIndex = (speedIndex + 1) % NUM_SPEEDS;
        switch (speedIndex) {
            case 0: DisplayPrint(ALARM, 1, "Speed: SLOW"); break;
            case 1: DisplayPrint(ALARM, 1, "Speed: MED");  break;
            case 2: DisplayPrint(ALARM, 1, "Speed: FAST"); break;
        }
    }
    prevSelect = currSelect;

    static bool prevStart = false;
    bool currStart = (input & BTN_START_BIT);
    if (currStart && !prevStart) {
        // Randomize first serve
        Time_t seed = TimeNow();
        P1serve = (seed % 2);
        state = SERVE;
        DisplayColor(ALARM, P1serve ? CYAN : YELLOW);
        DisplayPrint(ALARM, 0, P1serve ? "1P SERVES" : "2P SERVES");
        DisplayPrint(ALARM, 1, "Press Paddle");
        position = P1serve ? LEFT_EDGE : RIGHT_EDGE;
        GPIO_PortOutput(GPIOX, 1 << position);
    }
    prevStart = currStart;
} break;

// --------------------------------------------------------
// SERVE: Wait for correct paddle press, show score if select held
// --------------------------------------------------------
case SERVE: {
    bool selectHeld = (input & BTN_SELECT_BIT);
    if (selectHeld) {
        DisplayColor(ALARM, RED);
        DisplayPrint(ALARM, 0, "Score");
        char scoreLine[16];
        sprintf(scoreLine, "%02d - %02d", P1score, P2score);
        DisplayPrint(ALARM, 1, scoreLine);
        uint8_t leds = ((P1score & 0x0F) << 4) | (P2score & 0x0F);
        GPIO_PortOutput(GPIOX, leds);
    } else {
        GPIO_PortOutput(GPIOX, 1 << position);
    }

    bool P1press = (input & BTN_P1_MASK);
    bool P2press = (input & BTN_P2_MASK);

    if ((P1serve && P1press) || (!P1serve && P2press)) {
        direction = P1serve ? 0 : 1;
        position += P1serve ? +1 : -1;
        DisplayColor(ALARM, WHITE);
        DisplayPrint(ALARM, 0, "PLAY!");
        DisplayPrint(ALARM, 1, "");
        GPIO_PortOutput(GPIOX, 1 << position);
        timeShift = TimeNow();
        state = PLAY;
    }
} break;

// --------------------------------------------------------
// PLAY: Ball moves, check returns, handle scoring
// --------------------------------------------------------
case PLAY: {
    if (TimePassed(timeShift) >= speedTable[speedIndex]) {
        position += direction ? -1 : +1;
        GPIO_PortOutput(GPIOX, (position >= 0 && position <= 7) ? (1 << position) : 0x00);
        timeShift = TimeNow();
    }

    bool P1press = (input & BTN_P1_MASK);
    bool P2press = (input & BTN_P2_MASK);

    if (position == 1 || position == 2) {
        if (P1press) {
            direction = 0;
            DisplayColor(ALARM, CYAN);
        }
    }

    if (position == 5 || position == 6) {
        if (P2press) {
            direction = 1;
            DisplayColor(ALARM, YELLOW);
        }
    }

    if (position < LEFT_EDGE) {
        P1score++;
        DisplayColor(ALARM, CYAN);
        DisplayPrint(ALARM, 0, "1P SCORES!");
        char score[16]; sprintf(score, "%02d - %02d", P1score, P2score);
        DisplayPrint(ALARM, 1, score);
        state = SERVE;
        P1serve = (P1score + P2score) % 2 == 0 ? !P1serve : P1serve;
        position = P1serve ? LEFT_EDGE : RIGHT_EDGE;
        GPIO_PortOutput(GPIOX, 1 << position);
    } else if (position > RIGHT_EDGE) {
        P2score++;
        DisplayColor(ALARM, YELLOW);
        DisplayPrint(ALARM, 0, "2P SCORES!");
        char score[16]; sprintf(score, "%02d - %02d", P1score, P2score);
        DisplayPrint(ALARM, 1, score);
        state = SERVE;
        P1serve = (P1score + P2score) % 2 == 0 ? !P1serve : P1serve;
        position = P1serve ? LEFT_EDGE : RIGHT_EDGE;
        GPIO_PortOutput(GPIOX, 1 << position);
    }

    if ((P1score >= 11 || P2score >= 11) && (P1score - P2score >= 2 || P2score - P1score >= 2)) {
        state = WIN;
    }
} break;

// --------------------------------------------------------
// WIN: Flash LEDs, display winner, return on Start
// --------------------------------------------------------
case WIN: {
    if (P1score > P2score) {
        DisplayColor(ALARM, CYAN);
        DisplayPrint(ALARM, 0, "PLAYER 1 WINS!");
    } else {
        DisplayColor(ALARM, YELLOW);
        DisplayPrint(ALARM, 0, "PLAYER 2 WINS!");
    }
    char finalScore[16];
    sprintf(finalScore, "%02d - %02d", P1score, P2score);
    DisplayPrint(ALARM, 1, finalScore);

    static Time_t flashTime = 0;
    static bool ledsOn = false;
    if (TimePassed(flashTime) >= 400) {
        ledsOn = !ledsOn;
        GPIO_PortOutput(GPIOX, ledsOn ? 0xFF : 0x00);
        flashTime = TimeNow();
        }

    if (input & BTN_START_BIT) {
        P1score = 0; P2score = 0;
        firstServe = true;
        P1serve = true;
        position = 0;
        direction = 0;
        DisplayColor(ALARM, WHITE);
        DisplayPrint(ALARM, 0, "Linear Pong");
        DisplayPrint(ALARM, 1, "Press Start");
        GPIO_PortOutput(GPIOX, 1 << position);
        timeShift = TimeNow();
        state = TITLE;
        }
    } break;
  }
}
