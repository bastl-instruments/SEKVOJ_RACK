/*
 * SekvojRackSDPreset.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: dasvaclav
 */

#include "SekvojRackSDPreset.h"
#include <SdFat.h>
#include <SdFatUtil.h>
Sd2Card card;           // SD/SDHC card with support for version 2.00 features
SdVolume vol;           // FAT16 or FAT32 volume
SdFile root;            // volume's root directory
SdFile file;

SekvojRackSDPreset::SekvojRackSDPreset(){

}
char presetFileName[7]="PT.txt";
void SekvojRackSDPreset::initCard(unsigned char * data){

	if (!card.init()){/*Serial.println("int");*/};//error("card");
	if (!vol.init(&card)){/*Serial.println("crd");*/};// error("vol ");
	if (!root.openRoot(&vol)){/*Serial.println("vol");*/};// error("root");
	if (!file.open(&root, presetFileName, O_READ )) {
		//Serial.println("noFile");
		if (!file.open(&root, presetFileName, O_RDWR | O_CREAT )) {
			//Serial.println("errMake");
		} else {
			//Serial.println("makingNewFile");
			for (int j = 0; j < 64; j++) {
				for (int i = 0; i < 290; i++) file.print(data[i]);
				for (int i = 0; i < 222; i++) file.print(255);
				//Serial.print(".");
				//Serial.println(file.getC());
			}
		}

	} else if (!file.open(&root, presetFileName, O_RDWR | O_CREAT )) {}
}

void SekvojRackSDPreset::getPatternData(unsigned char patternIndex, unsigned char * data) {
	currentPattern=patternIndex;
	uint32_t positionInFile=patternIndex*512;
	file.seekSet(positionInFile);
	file.read(&data[0],290);
}

void SekvojRackSDPreset::setPatternData(unsigned char patternIndex, unsigned char * data) {
	uint32_t positionInFile=patternIndex*512;
	file.seekSet(positionInFile);
	file.write(&data[0],290);

}
