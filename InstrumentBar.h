/*
 * InstrumentBar.h
 *
 *  Created on: Sep 3, 2014
 *      Author: bastl
 */

#ifndef INSTRUMENTBAR_H_
#define INSTRUMENTBAR_H_

#include <IHWLayer.h>
#include "SekvojButtonMap.h"

#define NUMBER_OF_INSTRUMENTS 10

class InstrumentBar {
public:
	InstrumentBar();
	void init(IHWLayer * hw, SekvojButtonMap * buttonMap_);
	void setActive(bool isActive);
	void setInstrumentSelected(unsigned char instrumentIndex, bool isSelected);
	void setInstrumentPlaying(unsigned char instrumentIndex, bool isPlaying);
	void resetSelected();
private:
	IHWLayer * hw_;
	SekvojButtonMap * buttonMap_;
	int currentSelectedStatuses_;
	int currentPlayingStatuses_;
	bool isActive_;
};

#endif /* INSTRUMENTBAR_H_ */
