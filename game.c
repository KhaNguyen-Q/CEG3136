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
#include <stdio.h>

#define PONG_TIME_DEFAULT 500  // default ball speed

static Time_t game_time = 0;      // time for LED shifts
static Time_t select_press_time = 0; // for speed adjustment debounce

static int position = 0;  // LED index
static int direction = 0; // 0 = right, 1 = left
static int PONG_TIME = PONG_TIME_DEFAULT;

static int scoreP1 = 0;   // left player
static int scoreP2 = 0;   // right player

static enum { TITLEPONG, GAMEPONG, ENDPONG } state;

// --- helper: small integer to string ---
static void IntToStr(int num, char *str) {
    str[0] = '0' + (num / 10);
    str[1] = '0' + (num % 10);
    str[2] = '\0';
}

// -------------------------------------------------------------------

void Init_Game(void) {
    GPIO_PortEnable(GPIOX);
    DisplayEnable();
    DisplayColor(WHITE);

    state = TITLEPONG;
    DisplayPrint(0, "Linear Pong");
    DisplayPrint(1, "Press Start");
    position = 0;
    direction = 0;
    scoreP1 = 0;
    scoreP2 = 0;
    game_time = TimeNow();
    GPIO_PortOutput(GPIOX, 1 << position);
    UpdateDisplay();
}

// -------------------------------------------------------------------

void Task_Game(void) {
    uint32_t input = GPIO_PortInput(GPIOX);

    switch (state) {
        // ------------------- TITLE SCREEN -------------------
        case TITLEPONG:
            DisplayPrint(0, "PONG");
            DisplayPrint(1, "Press Start");

            // adjust speed (Select)
            if ((input & (1 << (3 + 8))) && TimePassed(select_press_time) > 800) {
                select_press_time = TimeNow();
                switch (PONG_TIME) {
                    case 250:
                        PONG_TIME = 500;
                        DisplayPrint(1, "Speed: MED");
                        break;
                    case 500:
                        PONG_TIME = 1000;
                        DisplayPrint(1, "Speed: SLOW");
                        break;
                    case 1000:
                        PONG_TIME = 250;
                        DisplayPrint(1, "Speed: FAST");
                        break;
                }
                UpdateDisplay();
            }

            // start game (Start)
            if ((input & (1 << (4 + 8)))) {
                GPIO_PortOutput(GPIOX, 0x00);
                position = 1;
                direction = 0;
                game_time = TimeNow();
                DisplayPrint(0, "Pong Playing");
                DisplayPrint(1, "Score 00:00");
                state = GAMEPONG;
                UpdateDisplay();
            }
            break;

        // ------------------- GAMEPLAY -------------------
        case GAMEPONG:
            if (TimePassed(game_time) > PONG_TIME) {
                // check edge -> score
                if (position == 7) {
                    scoreP1++;
                    state = ENDPONG;
                } else if (position == 0) {
                    scoreP2++;
                    state = ENDPONG;
                }

                // paddle check for bounce
                if (position == 1 || position == 2) {
                    if (input & ((1 << (0 + 8)) | (1 << (1 + 8)) | (1 << (2 + 8))))
                        direction = 0; // bounce right
                }
                if (position == 5 || position == 6) {
                    if (input & ((1 << (5 + 8)) | (1 << (6 + 8)) | (1 << (7 + 8))))
                        direction = 1; // bounce left
                }

                // update position
                position += direction ? -1 : +1;
                GPIO_PortOutput(GPIOX, 1 << position);
                game_time = TimeNow();
            }
            UpdateDisplay();
            break;

        // ------------------- GAME END (score update) -------------------
        case ENDPONG: {
            char left[3], right[3], line[16];
            IntToStr(scoreP1, left);
            IntToStr(scoreP2, right);

            // "Score xx:yy"
            line[0]='S'; line[1]='c'; line[2]='o'; line[3]='r'; line[4]='e';
            line[5]=' '; line[6]=left[0]; line[7]=left[1];
            line[8]=':'; line[9]=right[0]; line[10]=right[1]; line[11]='\0';

            DisplayPrint(0, "Round Over");
            DisplayPrint(1, line);

            // reset lights after point
            GPIO_PortOutput(GPIOX, 0x00);

            // press start to continue
            if ((input & (1 << (4 + 8)))) {
                state = TITLEPONG;
                position = 0;
                direction = 0;
                game_time = TimeNow();
            }
            UpdateDisplay();
            break;
        }
    }
}
