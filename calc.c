/*
 * calc.c
 *
 *  Created on: Nov 3, 2025
 *      Author: knguy138
 */


//calculator app
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "calc.h"
#include "display.h"
#include "touchpad.h"
#include "systick.h"

#define NMAX 10 // Max num of operands

static Press_t operation; // Selected operation

static Entry_t operand[NMAX]; // Numeric operands

static Entry_t arr[10]; //array for operations

static int count; // Count of operands received

static Entry_t result; // Calculation result

static Time_t time; //time used for displaying the arrays

static int counter = 0; //used for counting through array

static enum {MENU, PROMPT, ENTRY, ARRAYENTRY, ARRAYENTRY10, RUN, SHOW, SHOWFLOAT, SHOWARR, WAIT} state;

// Assembly subroutines to be called as functions
uint32_t Increment(uint32_t n);

uint32_t Decrement(uint32_t n);

uint32_t Classic4Function(uint32_t sel, uint32_t n1, uint32_t n2);

uint32_t Factorial(uint32_t n);

uint32_t Fibonacci(uint32_t n);

uint32_t GCD(uint32_t n, uint32_t n2);

uint32_t Sort(uint32_t n, uint32_t *arr);

uint32_t Average(uint32_t n, uint32_t *arr);






// Initialization
void Init_Calc (void) {
DisplayEnable();
TouchEnable();

DisplayPrint(CALC, 0, "Calculator App");
DisplayPrint(CALC, 1, "ENTER OP (0-9)");

time = TimeNow();
}

// Runtime
void Task_Calc (void) {
	switch (state) {
	case MENU: //menu screen
		// Prepare for a new operation
		operation = NONE;
		for (int i = 0; i < NOMAX; i++) //set all operands to 0
			operand[i] = 0;
		count = 0;
		result = 0;

		// Display menu

		state = PROMPT;
		break;

		case PROMPT: //check operation and go to correct routine
		if (operation == NONE)
			operation = TouchInput(CALC);
			switch ((int)operation) {

		case 1: // Increment
			if (count == 1)
				state = RUN;
			else {
				DisplayPrint(CALC, 0, "INC NUM:");
				state = ENTRY;
			}
			break;

		case 2: // add
			if (count == 2) //gets two operands
				state = RUN;
			else if (count == 1){
				DisplayPrint(CALC, 0, "ADD number 2:");
				state = ENTRY;
			}
			else if (count == 0){
				DisplayPrint(CALC, 0, "ADD number 1:");
				state = ENTRY;
			}
			break;

		case 3: //Subtract
			if (count == 2) //gets two operands
				state = RUN;
			else if (count == 1){
				DisplayPrint(CALC, 0, "SUB number 2:");
				state = ENTRY;
			}
			else if (count == 0){
				DisplayPrint(CALC, 0, "SUB number 1:");
				state = ENTRY;
			}
			break;

		case 4: //Multiply
			if (count == 2) //gets two operands
				state = RUN;
			else if (count == 1){
				DisplayPrint(CALC, 0, "MULT number 2:");
				state = ENTRY;
			}
			else if (count == 0){
				DisplayPrint(CALC, 0, "MULT number 1:");
				state = ENTRY;
			}
			break;

		case 5: //divide
			if (count == 2) //gets two operands
				state = RUN;
			else if (count == 1){
				DisplayPrint(CALC, 0, "DIV number 2:");
				state = ENTRY;
			}
			else if (count == 0){
				DisplayPrint(CALC, 0, "DIV number 1:");
				state = ENTRY;
			}
			break;

		case 6: //GCD
			if (count == 2) //gets two operands
				state = RUN;
			else if (count == 1){
				DisplayPrint(CALC, 0, "GCD number 2:");
				state = ENTRY;
			}
			else if (count == 0){
				DisplayPrint(CALC, 0, "GCD number 1:");
				state = ENTRY;
			}
			break;

		case 7: //FACTORIAL
			if (count == 1) //gets one operand
				state = RUN;
			else {
				DisplayPrint(CALC, 0, "FACTORIAL a num:");
				state = ENTRY;
			}
			break;

		case 8: //Fibonacci
			if (count == 1) //gets one operand
				state = RUN;
			else {
				DisplayPrint(CALC, 0, "FIBONACCI num:");
				state = ENTRY;
			}
			break;

		case 9: //sort

			if (count == 0){ //checks count as first input
			DisplayPrint(CALC, 0, "ENTR MAX SORT:");
			state = ENTRY;
			}
			else if (count < operand[0]+1){ //get all items
				DisplayPrint(CALC, 0, "Enter item %u:", count);
				state = ARRAYENTRY;
			}
			else {
				operand[1] = &arr;
				state = RUN;
			}
			break;

		case 0: //average;
			if (count == 0){ //checks count as first input
		DisplayPrint(CALC, 0, "ENTR NUM AVG:");
		state = ENTRY;
			}
			else if (count < operand[0]+1){ //get all items
				DisplayPrint(CALC, 0, "Enter item %u:", count);
				state = ARRAYENTRY10;
			}
			else {
				operand[1] = &arr;
				state = RUN;
			}
			break;
		default: // Do nothing
			state=MENU;
			break;
		}
		break;

		case ENTRY: //enter the operands
			bool done = TouchEntry(CALC, &operand[count]);
			DisplayPrint(CALC, 1, "%u", operand[count]);
			if (done) {
				count++;
				state = PROMPT;
			}
			break;

		case ARRAYENTRY: //enter values for the arrays
			bool done2 = TouchEntry(CALC, &arr[count-1]);
			DisplayPrint(CALC, 1, "%u", arr[count-1]);
			if (done2) {
				count++;
				state = PROMPT;
			}
			break;

		case ARRAYENTRY10: //enter values for the arrays, and multiply by 10.
			bool done3 = TouchEntry(CALC, &arr[count-1]);
			DisplayPrint(CALC, 1, "%u", arr[count-1]);

			if (done3) {
				arr[count-1] = arr[count-1] * 10;
				count++;
				state = PROMPT;
			}
			break;

		case RUN: //actually do the calculations
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			state = SHOW; //default we go to SHOW

			switch ((int)operation) {
			case 1: result = Increment(operand[0]); break;
			case 2: result = Classic4Function(0b0, operand[0], operand[1]); break;
			case 3: result = Classic4Function(0b1, operand[0], operand[1]); break;
			case 4: result = Classic4Function(0b10, operand[0], operand[1]); break;
			case 5: result = Classic4Function(0b11, operand[0], operand[1]); break;
			case 6: result = GCD(operand[0], operand[1]); break;
			case 7: result = Factorial(operand[0]); break;
			case 8: result = Fibonacci(operand[0]); break;
			case 9:
				result = Sort(operand[0], arr);
				state = SHOWARR; //we show array
				counter = 0;
				break;

			case 0:
				result = Average(operand[0], arr);
				state = SHOWFLOAT; //we show float
				for (int i = 0; i< operand[0]; i++) { //return array to 0 for next operation
					arr[i] = 0;
				}
				break;
			}
			break;

			case SHOW: //display the already calculated results on the screen
				DisplayPrint(CALC, 0, "Result:");
				DisplayPrint(CALC, 1, "%u", result);
				state = WAIT;
				break;
			case SHOWFLOAT: //display the already calculated results on the screen for a float
				DisplayPrint(CALC, 0, "Result:");
				DisplayPrint(CALC, 1, "%u.%u", result / 10, result % 10);
				state = WAIT;
				break;
			case SHOWARR: //display the already calculated results on the screen for an array
				DisplayPrint(CALC, 0, "Result:");

				if (TimePassed(time) > 750) { //display the ith element of the array every 750 ms
					time = TimeNow();
				if (counter < operand[0]) {
					DisplayPrint(CALC, 1, "Item %u: %u", counter+1, arr[counter]);
				}
				else {
					state = MENU;
					DisplayPrint(CALC, 0, "Calculator App");
					DisplayPrint(CALC, 1, "ENTER OP (0-9)");
					for (int i = 0; i< operand[0]; i++) { //return array to 0 for next operation
						arr[i] = 0;
					}
					break;
				}
				counter++;
				}


				break;

			case WAIT: //wait for next button press to return to menu
				// Press any pad to return to the menu
				if (TouchInput(CALC) != NONE) {
					state = MENU;
					DisplayPrint(CALC, 0, "Calculator App");
					DisplayPrint(CALC, 1, "ENTER OP (0-9)");
				}
				break;
			}
	}


