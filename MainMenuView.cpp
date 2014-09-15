/*
 * MainMenuView.cpp
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#include "MainMenuView.h"
#include "SetActiveView.h"
#include "PatternView.h"
#include "SetStepView.h"
#include "PlayRecordView.h"

MainMenuView::MainMenuView() : hw_(0), player_(0), recorder_(0), memory_(0), settings_(0), midiProcessor_(0),
							   instrumentBar_(0), buttonMap_(0), currentView_(0), currentViewIndex_(0), currentPattern_(0),
							   modeButtons_(0) {
}

MainMenuView::~MainMenuView() {
	delete modeButtons_;
}

void MainMenuView::init(IHWLayer * hw, Player * player, StepRecorder * recorder,
						IStepMemory * memory, PlayerSettings * settings, IMIDICommandProcessor * midiProcessor,
						InstrumentBar * instrumentBar, SekvojButtonMap * buttonMap) {
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
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(0), IHWLayer::ON);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(1), IHWLayer::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(2), IHWLayer::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(3), IHWLayer::OFF);
}

void MainMenuView::createView(unsigned char viewIndex) {
	switch (viewIndex) {
	case 0: {
		SetActiveView * activeView = new SetActiveView();
		activeView->init(hw_, memory_, player_, instrumentBar_, buttonMap_, settings_->getCurrentPattern());
		currentView_ = (IView*)activeView;
		break;
	}
	case 1: {
		PatternView * patternView = new PatternView();
		patternView->init(hw_, settings_, memory_, instrumentBar_, buttonMap_);
		currentView_ = (IView*)patternView;
		break;
	}
	case 2: {
		SetStepView * setStepView = new SetStepView();
		setStepView->init(hw_, memory_, player_, instrumentBar_, buttonMap_, settings_->getCurrentPattern());
		currentView_ = (IView*)setStepView;
		break;
	}
	case 3:
		PlayRecordView * playRecordView = new PlayRecordView();
		playRecordView->init(hw_, recorder_, buttonMap_);
		currentView_ = (IView*)playRecordView;
		break;
	}
}

void MainMenuView::update() {
	modeButtons_->update();
	unsigned char newIndex = 0;
	bool buttonSelected = modeButtons_->getSelectedButton(newIndex);
	if (buttonSelected && newIndex != currentViewIndex_) {
		for (int i = 0; i < 36; i++) {
			hw_->setLED(buttonMap_->getButtonIndex(i), IHWLayer::OFF);
		}
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(currentViewIndex_), IHWLayer::OFF);
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(newIndex), IHWLayer::ON);
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


