/*
 * SekvojRackButtonMap.h
 *
 *  Created on: Sep 3, 2014
 *      Author: bastl
 */

#ifndef SEKVOJRACKBUTTONMAP_H_
#define SEKVOJRACKBUTTONMAP_H_

#include <IButtonMap.h>

class SekvojRackButtonMap : public IButtonMap {
public:
	SekvojRackButtonMap();
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
	unsigned char getPlayButtonIndex();
	unsigned char getRecordButtonIndex();
	unsigned char getJumpButtonIndex();
	unsigned char getFunctionButtonIndex();
	unsigned char getPatternButtonIndex();
	unsigned char getActiveButtonIndex();

private:
	unsigned char buttonIndexes_[32];
};

inline unsigned char SekvojRackButtonMap::getButtonIndex(unsigned char index) {
	return buttonIndexes_[4 + index];
}

inline unsigned char SekvojRackButtonMap::getMainMenuButtonIndex(unsigned char index) {
	return buttonIndexes_[20 + index];
}

inline unsigned char SekvojRackButtonMap::getInstrumentButtonIndex(unsigned char index) {
	return buttonIndexes_[26 + index];
}

inline unsigned char SekvojRackButtonMap::getStepButtonIndex(unsigned char index) {
	return buttonIndexes_[4 + index];
}

inline unsigned char SekvojRackButtonMap::getSubStepButtonIndex(unsigned char index) {
	return buttonIndexes_[index];
}

inline unsigned char SekvojRackButtonMap::getVelocityButtonIndex(unsigned char index) {
	return buttonIndexes_[index];
}

inline unsigned char* SekvojRackButtonMap::getMainMenuButtonArray() {
	return &buttonIndexes_[20];
}

inline unsigned char* SekvojRackButtonMap::getInstrumentButtonArray() {
	return &buttonIndexes_[26];
}

inline unsigned char* SekvojRackButtonMap::getStepButtonArray() {
	return &buttonIndexes_[4];
}

inline unsigned char* SekvojRackButtonMap::getSubStepButtonArray() {
	return &buttonIndexes_[0];
}

inline unsigned char* SekvojRackButtonMap::getVelocityButtonArray() {
	return &buttonIndexes_[0];
}

inline unsigned char SekvojRackButtonMap::getPlayButtonIndex() {
	return 18;
}
inline unsigned char SekvojRackButtonMap::getRecordButtonIndex() {
	return 26;
}

inline unsigned char SekvojRackButtonMap::getJumpButtonIndex() {
	return 24;
}

inline unsigned char SekvojRackButtonMap::getFunctionButtonIndex() {
	return 17;
}

inline unsigned char SekvojRackButtonMap::getPatternButtonIndex() {
	return 25;
}

inline unsigned char SekvojRackButtonMap::getActiveButtonIndex() {
	return 16;
}

#endif /* SEKVOJRACKBUTTONMAP_H_ */

