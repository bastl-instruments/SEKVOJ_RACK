/*
 * SekvojRackButtonMap.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: bastl
 */

#include <SekvojRackButtonMap.h>

SekvojRackButtonMap::SekvojRackButtonMap() {

	buttonIndexes_[0] = 3;
	buttonIndexes_[1] = 11;
	buttonIndexes_[2] = 19;
	buttonIndexes_[3] = 27;

	buttonIndexes_[4] = 4;
	buttonIndexes_[5] = 12;
	buttonIndexes_[6] = 20;
	buttonIndexes_[7] = 28;
	buttonIndexes_[8] = 5;
	buttonIndexes_[9] = 13;
	buttonIndexes_[10] = 21;
	buttonIndexes_[11] = 29;
	buttonIndexes_[12] = 6;
	buttonIndexes_[13] = 14;
	buttonIndexes_[14] = 22;
	buttonIndexes_[15] = 30;
	buttonIndexes_[16] = 7;
	buttonIndexes_[17] = 15;
	buttonIndexes_[18] = 23;
	buttonIndexes_[19] = 31;

	buttonIndexes_[20] = 17; //fn
	buttonIndexes_[21] = 16; //active
	buttonIndexes_[22] = 18; //play
	buttonIndexes_[23] = 26; //rec
	buttonIndexes_[24] = 25; //pattern
	buttonIndexes_[25] = 24; //jump

	buttonIndexes_[26] = 1;
	buttonIndexes_[27] = 0;
	buttonIndexes_[28] = 2;
	buttonIndexes_[29] = 9;
	buttonIndexes_[30] = 8;
	buttonIndexes_[31] = 10;
}
