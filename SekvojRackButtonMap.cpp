/*
 * SekvojRackButtonMap.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: bastl
 */

#include <SekvojRackButtonMap.h>

SekvojRackButtonMap::SekvojRackButtonMap() {

	buttonIndexes_[FIRST_STEP_BUTTON_INDEX] = 3;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 1] = 11;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 2] = 19;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 3] = 27;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 4] = 2;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 5] = 10;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 6] = 18;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 7] = 26;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 8] = 1;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 9] = 9;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 10] = 17;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 11] = 25;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 12] = 0;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 13] = 8;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 14] = 16;
	buttonIndexes_[FIRST_STEP_BUTTON_INDEX + 15] = 24;

	buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX] = 22;
	buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX + 1] = 23;
	buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX + 2] = 21;
	buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX + 4] = 30;
	buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX + 5] = 31;
	buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX + 6] = 29;

	buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX] = 6;
	buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX + 1] = 14;
	buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX + 2] = 7;
	buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX + 3] = 15;
	buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX + 4] = 5;
	buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX + 5] = 13;

	buttonIndexes_[FIRST_SUBSTEP_BUTTON_INDEX] = 4;
	buttonIndexes_[FIRST_SUBSTEP_BUTTON_INDEX + 1] = 12;
	buttonIndexes_[FIRST_SUBSTEP_BUTTON_INDEX + 2] = 20;
	buttonIndexes_[FIRST_SUBSTEP_BUTTON_INDEX + 3] = 28;
}
