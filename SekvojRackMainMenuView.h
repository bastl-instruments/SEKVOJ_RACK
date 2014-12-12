/*
 * SekvojRackMainMenuView.h
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#ifndef SEKVOJRACKMAINMENUVIEW_H_
#define SEKVOJRACKMAINMENUVIEW_H_

#include <IView.h>
#include "sekvojHW.h"
#include <Player.h>
#include <IStepMemory.h>
#include <RadioButtons.h>
#include <PlayerSettings.h>
#include <BastlMetronome.h>
#include <StepRecorder.h>
#include "InstrumentBar.h"
#include <IButtonMap.h>

class SekvojRackMainMenuView : public IView {
public:
	SekvojRackMainMenuView();
	~SekvojRackMainMenuView();
	void init(sekvojHW * hw, Player * player, StepRecorder * recorder,
			  IStepMemory * memory, PlayerSettings * settings, IMIDICommandProcessor * midiProcessor,
			  InstrumentBar * instrumentBar, IButtonMap * buttonMap);
	void update();
private:

	sekvojHW * hw_;
	Player * player_;
	StepRecorder * recorder_;
	IStepMemory * memory_;
	PlayerSettings * settings_;
	IMIDICommandProcessor * midiProcessor_;
	InstrumentBar * instrumentBar_;
	IButtonMap * buttonMap_;

	IView * currentView_;
	unsigned char currentViewIndex_;
	unsigned char currentPattern_;

	RadioButtons * modeButtons_;

	void createView(unsigned char viewIndex);

};

#endif /* SEKVOJRACKMAINMENUVIEW_H_ */
