/*
 * SekvojRackMainMenuView.cpp
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#include "SekvojRackMainMenuView.h"
#include <SetActiveView.h>
#include <PatternView.h>
#include <SetStepView.h>
#include <PlayRecordView.h>

SekvojRackMainMenuView::SekvojRackMainMenuView() : hw_(0), player_(0), recorder_(0), memory_(0), settings_(0), midiProcessor_(0),
							   instrumentBar_(0), buttonMap_(0), currentView_(0), currentViewIndex_(0), currentPattern_(0),
							   modeButtons_(0) {
}

SekvojRackMainMenuView::~SekvojRackMainMenuView() {
	delete modeButtons_;
}

void SekvojRackMainMenuView::init(sekvojHW * hw, Player * player, StepRecorder * recorder,
						IStepMemory * memory, PlayerSettings * settings, IMIDICommandProcessor * midiProcessor,
						InstrumentBar * instrumentBar, IButtonMap * buttonMap) {
	hw_ = hw;
	player_ = player;
	recorder_ = recorder;
	memory_ = memory;
	settings_ = settings;
	midiProcessor_ = midiProcessor;
	instrumentBar_ = instrumentBar;
	buttonMap_ = buttonMap;

	modeButtons_ = new RadioButtons(hw_, buttonMap_->getMainMenuButtonArray(), 4);
	createView(currentViewIndex_);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(0), ILEDHW::ON);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(1), ILEDHW::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(2), ILEDHW::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(3), ILEDHW::OFF);
}

void SekvojRackMainMenuView::createView(unsigned char viewIndex) {
	switch (viewIndex) {
	case 3: {
		SetActiveView * activeView = new SetActiveView();
		activeView->init(hw_, memory_, player_, instrumentBar_, buttonMap_, settings_->getCurrentPattern());
		currentView_ = (IView*)activeView;
		break;
	}
	case 2: {
		PatternView * patternView = new PatternView();
		patternView->init(hw_, settings_, memory_, instrumentBar_, buttonMap_);
		currentView_ = (IView*)patternView;
		break;
	}
	case 0: {
		SetStepView * setStepView = new SetStepView();
		setStepView->init(hw_, memory_, player_, instrumentBar_, buttonMap_, settings_->getCurrentPattern(), 6, false);
		currentView_ = (IView*)setStepView;
		break;
	}
	case 1:
		PlayRecordView * playRecordView = new PlayRecordView();
		playRecordView->init(hw_, recorder_, buttonMap_);
		currentView_ = (IView*)playRecordView;
		break;
	}
}

void SekvojRackMainMenuView::update() {
	modeButtons_->update();
	unsigned char newIndex = 0;
	bool buttonSelected = modeButtons_->getSelectedButton(newIndex);
	if (buttonSelected && newIndex != currentViewIndex_) {
		for (int i = 0; i < 32; i++) {
			hw_->setLED(buttonMap_->getButtonIndex(i), ILEDHW::OFF);
		}
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(currentViewIndex_), ILEDHW::OFF);
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(newIndex), ILEDHW::ON);
		currentViewIndex_ = newIndex;
		if (currentView_ != 0) {
			delete currentView_;
			currentView_ = 0;
		}
		instrumentBar_->resetSelected();
		createView(currentViewIndex_);
		return;
	}
	if (currentView_) {
		currentView_->update();
	}
}


