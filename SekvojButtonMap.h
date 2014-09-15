/*
 * SekvojButtonMap.h
 *
 *  Created on: Sep 3, 2014
 *      Author: bastl
 */

#ifndef SEKVOJBUTTONMAP_H_
#define SEKVOJBUTTONMAP_H_


#define FIRST_STEP_BUTTON_INDEX 0
#define FIRST_INSTRUMENT_BUTTON_INDEX FIRST_STEP_BUTTON_INDEX + 16
#define FIRST_MAIN_MENU_BUTTON_INDEX FIRST_INSTRUMENT_BUTTON_INDEX + 10
#define FIRST_VELOCITY_BUTTON_INDEX FIRST_MAIN_MENU_BUTTON_INDEX + 4
#define FIRST_SUBSTEP_BUTTON_INDEX FIRST_VELOCITY_BUTTON_INDEX + 2

class SekvojButtonMap {
public:
	SekvojButtonMap();
	unsigned char getButtonIndex(unsigned char index);
	unsigned char getMainMenuButtonIndex(unsigned char index);
	unsigned char getInstrumentButtonIndex(unsigned char index);
	unsigned char getStepButtonIndex(unsigned char index);
	unsigned char getSubStepButtonIndex(unsigned char index);
	unsigned char getVelocityButtonIndex(unsigned char index);
	unsigned char * getMainMenuButtonArray();
	unsigned char * getInstrumentButtonArray();
	unsigned char * getStepButtonArray();
	unsigned char * getSubStepButtonArray();
	unsigned char * getVelocityButtonArray();
private:
	unsigned char buttonIndexes_[36];
};

inline unsigned char SekvojButtonMap::getButtonIndex(unsigned char index) {
	return buttonIndexes_[index];
}

inline unsigned char SekvojButtonMap::getMainMenuButtonIndex(unsigned char index) {
	return buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX + index];
}

inline unsigned char SekvojButtonMap::getInstrumentButtonIndex(unsigned char index) {
	return buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX + index];
}

inline unsigned char SekvojButtonMap::getStepButtonIndex(unsigned char index) {
	return buttonIndexes_[FIRST_STEP_BUTTON_INDEX + index];
}

inline unsigned char SekvojButtonMap::getSubStepButtonIndex(unsigned char index) {
	return buttonIndexes_[FIRST_SUBSTEP_BUTTON_INDEX + index];
}

inline unsigned char SekvojButtonMap::getVelocityButtonIndex(unsigned char index) {
	return buttonIndexes_[FIRST_VELOCITY_BUTTON_INDEX + index];
}

inline unsigned char* SekvojButtonMap::getMainMenuButtonArray() {
	return &buttonIndexes_[FIRST_MAIN_MENU_BUTTON_INDEX];
}

inline unsigned char* SekvojButtonMap::getInstrumentButtonArray() {
	return &buttonIndexes_[FIRST_INSTRUMENT_BUTTON_INDEX];
}

inline unsigned char* SekvojButtonMap::getStepButtonArray() {
	return &buttonIndexes_[FIRST_STEP_BUTTON_INDEX];
}

inline unsigned char* SekvojButtonMap::getSubStepButtonArray() {
	return &buttonIndexes_[FIRST_SUBSTEP_BUTTON_INDEX];
}

inline unsigned char* SekvojButtonMap::getVelocityButtonArray() {
	return &buttonIndexes_[FIRST_VELOCITY_BUTTON_INDEX];
}

#endif /* SEKVOJBUTTONMAP_H_ */

