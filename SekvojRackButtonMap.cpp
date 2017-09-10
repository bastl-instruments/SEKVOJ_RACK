/*
 * SekvojRackButtonMap.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: bastl
 */

#include <SekvojRackButtonMap.h>

SekvojRackButtonMap::SekvojRackButtonMap() {

	buttonIndexes_[0] = 4;
	buttonIndexes_[1] = 12;
	buttonIndexes_[2] = 20;
	buttonIndexes_[3] = 28;
	buttonIndexes_[4] = 5;
	buttonIndexes_[5] = 13;
	buttonIndexes_[6] = 21;
	buttonIndexes_[7] = 29;
	buttonIndexes_[8] = 6;
	buttonIndexes_[9] = 14;
	buttonIndexes_[10] = 22;
	buttonIndexes_[11] = 30;
	buttonIndexes_[12] = 7;
	buttonIndexes_[13] = 15;
	buttonIndexes_[14] = 23;
	buttonIndexes_[15] = 31;

	buttonIndexes_[16] = 17; //fn
	buttonIndexes_[17] = 16; //active
	buttonIndexes_[18] = 18; //play
	buttonIndexes_[19] = 26; //rec
	buttonIndexes_[20] = 25; //pattern
	buttonIndexes_[21] = 24; //jump

	buttonIndexes_[22] = 1;
	buttonIndexes_[23] = 0;
	buttonIndexes_[24] = 2;
	buttonIndexes_[25] = 9;
	buttonIndexes_[26] = 8;
	buttonIndexes_[27] = 10;

	buttonIndexes_[28] = 3;
	buttonIndexes_[29] = 11;
	buttonIndexes_[30] = 19;
	buttonIndexes_[31] = 27;
}
