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
#include <Switches.h>

#define MENU_PLAY_INDEX 4
#define MENU_RECORD_INDEX 5
#define MENU_TAP_INDEX 0
#define MENU_ERASE_INDEX 1
#define MENU_PATTERN_INDEX 2
#define MENU_ACTIVE_INDEX 3

class SekvojRackMainMenuView : public IView {
public:
	SekvojRackMainMenuView();
	~SekvojRackMainMenuView();
	void init(sekvojHW * hw, Player * player, StepRecorder * recorder,
			  IStepMemory * memory, PlayerSettings * settings, IMIDICommandProcessor * midiProcessor,
			  InstrumentBar * instrumentBar, IButtonMap * buttonMap);
	void update();
private:

	enum UIStatus{INIT, ACTIVE, RECORDING, PATTERN};

	sekvojHW * hw_;
	Player * player_;
	StepRecorder * recorder_;
	IStepMemory * memory_;
	PlayerSettings * settings_;
	IMIDICommandProcessor * midiProcessor_;
	InstrumentBar * instrumentBar_;
	IButtonMap * buttonMap_;
	Switches recordSwitch_;
	Switches activeSwitch_;

	IView * currentView_;
	unsigned char currentViewIndex_;
	unsigned char currentPattern_;

	UIStatus currentStatus_;
	unsigned char selectedInstrument_;

	void createView(unsigned char viewIndex);
	void updateInInit();
	void updateInActive();
	void updateInRecording();
	void updateInPattern();
	void createSetStepView();

};

#endif /* SEKVOJRACKMAINMENUVIEW_H_ */
