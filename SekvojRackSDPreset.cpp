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

void SekvojRackSDPreset::initCard(){

	if (!card.init()){Serial.println("int");};//error("card");
		if (!vol.init(&card)){Serial.println("crd");};// error("vol ");
		if (!root.openRoot(&vol)){Serial.println("vol");};// error("root");
		Serial.println("redy");
	if (!file.open(&root, "PT.txt", O_RDWR | O_CREAT )) {
		}

}

void SekvojRackSDPreset::getPatternData(unsigned char patternIndex, unsigned char * data) {
	//timing needs optimisation ?
		// cca 60 ms opening file
			// cca 19 ms writing
		//cca 2 ms closing file
	currentPattern=patternIndex;
	Serial.println("load:");
	uint32_t positionInFile=patternIndex*512;
	uint32_t time=millis();
	file.seekSet(positionInFile);
			file.read(&data[0],290);
			/*
	if (!file.open(&root, "PT.txt", O_READ )) {

	}
	else{Serial.println(file.curPosition());
		file.seekSet(positionInFile);
		file.read(&data[0],290);
	}
	file.close();
*/
    Serial.println(millis()-time);
    //for (unsigned int dataIndex= 0; dataIndex < 288; dataIndex++) Serial.print(data[dataIndex]),Serial.print(" ,");
   // Serial.println();
}

void SekvojRackSDPreset::setPatternData(unsigned char patternIndex, unsigned char * data) {
Serial.println("store:");
	uint32_t positionInFile=patternIndex*512;
	uint32_t time=millis();
	file.seekSet(positionInFile);

				file.write(&data[0],290);
				/*
	if (!file.open(&root, "PT.txt", O_RDWR | O_CREAT )) {

	}
	else{
		//Serial.println(file.getFileSize());

			file.seekSet(positionInFile);

			file.write(&data[0],290);

	}
	file.close();
*/
    Serial.println(millis()-time);
   // for (unsigned int dataIndex= 0; dataIndex < 288; dataIndex++) Serial.print(data[dataIndex]),Serial.print(" ,");

}
