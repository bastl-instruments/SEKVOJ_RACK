/*
 * PatternView.h
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#ifndef PATTERNVIEW_H_
#define PATTERNVIEW_H_

#include <IView.h>
#include <IStepMemory.h>
#include <RadioButtons.h>
#include <Switches.h>
#include <PlayerSettings.h>
#include "InstrumentBar.h"
#include "SekvojButtonMap.h"

class PatternView  : public IView {
public:
	PatternView();
	~PatternView();
	void init(IHWLayer * hw, PlayerSettings * settigns, IStepMemory * memory, InstrumentBar * instrumentBar, SekvojButtonMap * buttonMap);
	void update();
private:

	IHWLayer * hw_;
	PlayerSettings * settings_;
	IStepMemory * memory_;
	InstrumentBar * instrumentBar_;
	SekvojButtonMap * buttonMap_;

	//Controls
	Switches instrumentSwitches_;
	RadioButtons * patternSelectRadioButtons_;

	unsigned int currentInstrumentStatuses_;
	unsigned char currentPattern_;

	void reflectPatternChange();
};

#endif /* PATTERNVIEW_H_ */
