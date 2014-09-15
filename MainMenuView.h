/*
 * MainMenuView.h
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#ifndef MAINMENUVIEW_H_
#define MAINMENUVIEW_H_

#include <IView.h>
#include <IHWLayer.h>
#include <Player.h>
#include <IStepMemory.h>
#include <RadioButtons.h>
#include <PlayerSettings.h>
#include <BastlMetronome.h>
#include <StepRecorder.h>
#include "InstrumentBar.h"
#include "SekvojButtonMap.h"

class MainMenuView : public IView {
public:
	MainMenuView();
	~MainMenuView();
	void init(IHWLayer * hw, Player * player, StepRecorder * recorder,
			  IStepMemory * memory, PlayerSettings * settings, IMIDICommandProcessor * midiProcessor,
			  InstrumentBar * instrumentBar, SekvojButtonMap * buttonMap);
	void update();
private:

	IHWLayer * hw_;
	Player * player_;
	StepRecorder * recorder_;
	IStepMemory * memory_;
	PlayerSettings * settings_;
	IMIDICommandProcessor * midiProcessor_;
	InstrumentBar * instrumentBar_;
	SekvojButtonMap * buttonMap_;

	IView * currentView_;
	unsigned char currentViewIndex_;
	unsigned char currentPattern_;

	RadioButtons * modeButtons_;

	void createView(unsigned char viewIndex);

};

#endif /* MAINMENUVIEW_H_ */
