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
#include <SettingsAndFunctionsView.h>
#include <IButtonHW.h>

SekvojRackMainMenuView::SekvojRackMainMenuView() : hw_(0), player_(0), recorder_(0), memory_(0), settings_(0), midiProcessor_(0),
							   instrumentBar_(0), buttonMap_(0), currentView_(0), currentViewIndex_(0), currentPattern_(0),
							   currentStatus_(INIT), selectedInstrument_(0) {
}

SekvojRackMainMenuView::~SekvojRackMainMenuView() {
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

	createSetStepView();

	hw_->setLED(buttonMap_->getMainMenuButtonIndex(0), ILEDHW::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(1), ILEDHW::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(2), ILEDHW::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(3), ILEDHW::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(4), ILEDHW::OFF);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(5), ILEDHW::OFF);

	unsigned char * recordButton = buttonMap_->getMainMenuButtonArray() + MENU_RECORD_INDEX;
	unsigned char * activeButton = buttonMap_->getMainMenuButtonArray() + MENU_ACTIVE_INDEX;

	recordSwitch_.init(hw_, recordButton, 1, IButtonHW::DOWN);
	activeSwitch_.init(hw_, activeButton, 1, IButtonHW::DOWN);
}

void SekvojRackMainMenuView::createSetStepView() {
	SetStepView * setStepView = new SetStepView();
	setStepView->init(hw_, memory_, player_, instrumentBar_, buttonMap_, settings_->getCurrentPattern(), 6, selectedInstrument_, false);
	currentView_ = (IView*)setStepView;
}

void SekvojRackMainMenuView::createView(unsigned char viewIndex) {
	switch (viewIndex) {
		case 2: {
			PatternView * patternView = new PatternView();
			patternView->init(hw_, settings_, memory_, instrumentBar_, buttonMap_);
			currentView_ = (IView*)patternView;
			break;
		}
	}
}

void SekvojRackMainMenuView::updateInInit() {
	recordSwitch_.update();
	activeSwitch_.update();
	if (recordSwitch_.getStatus(0)) {
		currentStatus_ = RECORDING;
		destroyInitView();
		PlayRecordView * playRecordView = new PlayRecordView();
		playRecordView->init(hw_, recorder_, buttonMap_);
		currentView_ = (IView*)playRecordView;
		instrumentBar_->resetSelected();
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_RECORD_INDEX), ILEDHW::ON);
		return;
	}
	if (activeSwitch_.getStatus(0)) {
		currentStatus_ = ACTIVE;
		destroyInitView();
		SetActiveView * activeView = new SetActiveView();
		activeView->init(hw_, memory_, player_, instrumentBar_, buttonMap_);
		currentView_ = (IView*)activeView;
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_ACTIVE_INDEX), ILEDHW::ON);
		return;
	}
	if (hw_->getButtonState(buttonMap_->getMainMenuButtonIndex(MENU_PATTERN_INDEX)) == IButtonHW::DOWN) {
		currentStatus_ = PATTERN;
		destroyInitView();
		PatternView * patternView = new PatternView();
		patternView->init(hw_, settings_, memory_, instrumentBar_, buttonMap_);
		currentView_ = (IView*)patternView;
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_PATTERN_INDEX), ILEDHW::ON);
		return;
	}
	if (hw_->getButtonState(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX)) == IButtonHW::DOWN) {
		currentStatus_ = FUNCTION;
		destroyInitView();
		SettingsAndFunctionsView * functionView = new SettingsAndFunctionsView();
		functionView->init(hw_, settings_,instrumentBar_, buttonMap_);
		currentView_ = (IView*)functionView;
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX), ILEDHW::ON);
		return;
	}
}

void SekvojRackMainMenuView::destroyInitView() {
	selectedInstrument_ = ((SetStepView *) currentView_)->getSelectedIndstrumentIndex();
	delete currentView_;
	for (int i = 0; i < 32; i++) {
		hw_->setLED(buttonMap_->getButtonIndex(i), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::updateInPattern() {
	if (hw_->getButtonState(buttonMap_->getMainMenuButtonIndex(MENU_PATTERN_INDEX)) == IButtonHW::UP) {
		currentStatus_ = INIT;
		delete currentView_;
		createSetStepView();
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_PATTERN_INDEX), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::updateInFunction() {
	if (hw_->getButtonState(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX)) == IButtonHW::UP) {
		currentStatus_ = INIT;
		delete currentView_;
		createSetStepView();
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::updateInActive() {
	activeSwitch_.update();
	if (!activeSwitch_.getStatus(0)) {
		currentStatus_ = INIT;
		delete currentView_;
		createSetStepView();
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_ACTIVE_INDEX), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::updateInRecording() {
	recordSwitch_.update();
	if (!recordSwitch_.getStatus(0)) {
		currentStatus_ = INIT;
		delete currentView_;
		createSetStepView();
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_RECORD_INDEX), ILEDHW::OFF);
	}
}




void SekvojRackMainMenuView::update() {

	switch (currentStatus_) {
		case INIT:
			updateInInit();
			break;
		case ACTIVE:
			updateInActive();
			break;
		case RECORDING:
			updateInRecording();
			break;
		case PATTERN:
			updateInPattern();
			break;
		case FUNCTION:
			updateInFunction();
			break;
	}

	/*unsigned char newIndex = 0;
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
	}*/
	if (currentView_) {
		currentView_->update();
	}
}


