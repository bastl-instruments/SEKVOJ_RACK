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
							   currentStatus_(INIT), selectedInstrument_(0), synchronizer_(0) {
}

SekvojRackMainMenuView::~SekvojRackMainMenuView() {
}

void SekvojRackMainMenuView::init(sekvojHW * hw, Player * player, StepRecorder * recorder,
						IStepMemory * memory, PlayerSettings * settings, IMIDICommandProcessor * midiProcessor,
						InstrumentBar * instrumentBar, IButtonMap * buttonMap, StepSynchronizer * synchronizer) {
	hw_ = hw;
	player_ = player;
	recorder_ = recorder;
	memory_ = memory;
	settings_ = settings;
	midiProcessor_ = midiProcessor;
	instrumentBar_ = instrumentBar;
	buttonMap_ = buttonMap;
	synchronizer_ = synchronizer;

	createSetStepView();

	for (unsigned char index = 0; index < 6; index ++) {
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(index), ILEDHW::OFF);
	}

	unsigned char * recordButton = buttonMap_->getMainMenuButtonArray() + MENU_PLAY_INDEX;
	unsigned char * activeButton = buttonMap_->getMainMenuButtonArray() + MENU_ACTIVE_INDEX;

	playRecordSwitch_.init(hw_, recordButton, 2, IButtonHW::DOWN);
	playRecordSwitch_.setStatus(0, true);
	activeSwitch_.init(hw_, activeButton, 1, IButtonHW::DOWN);
}

void SekvojRackMainMenuView::createSetStepView() {
	SetStepView * setStepView = new SetStepView();
	setStepView->init(hw_, memory_, player_, instrumentBar_, buttonMap_,
					  settings_->getCurrentPattern(), 6, selectedInstrument_, settings_);
	currentView_ = (IView*)setStepView;
	currentStatus_ = INIT;
}

void SekvojRackMainMenuView::createFunctionView(bool fromRecord) {
	currentStatus_ = fromRecord ?  FUNCTION_FROM_RECORD : FUNCTION;
	SettingsAndFunctionsView * functionView = new SettingsAndFunctionsView();
	functionView->init(hw_, settings_,instrumentBar_, buttonMap_, memory_, selectedInstrument_, player_);
	currentView_ = (IView*)functionView;
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX), ILEDHW::ON);
}

void SekvojRackMainMenuView::createRecordView() {
	currentStatus_ = RECORDING;
	PlayRecordView * playRecordView = new PlayRecordView();
	playRecordView->init(hw_, recorder_, buttonMap_, synchronizer_);
	currentView_ = (IView*)playRecordView;
	instrumentBar_->resetSelected();
	playRecordSwitch_.setStatus(1, true);
	hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_RECORD_INDEX), ILEDHW::ON);
}

void SekvojRackMainMenuView::updateInInit() {
	activeSwitch_.update();
	((SetStepView*)currentView_)->setPlaying(playRecordSwitch_.getStatus(0));
	if (playRecordSwitch_.getStatus(1)) {
		destroyInitView();
		createRecordView();
		return;
	}
	if (activeSwitch_.getStatus(0)) {
		currentStatus_ = ACTIVE;
		destroyInitView();
		SetActiveView * activeView = new SetActiveView();
		activeView->init(hw_, memory_, player_, instrumentBar_, buttonMap_, selectedInstrument_);
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
		destroyInitView();
		createFunctionView(false);
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
		delete currentView_;
		createSetStepView();
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_PATTERN_INDEX), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::updateInFunction() {
	if (hw_->getButtonState(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX)) == IButtonHW::UP) {
		delete currentView_;
		if (currentStatus_ == FUNCTION_FROM_RECORD) {
			createRecordView();
		} else {
			createSetStepView();
		}
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::updateInActive() {
	activeSwitch_.update();
	if (!activeSwitch_.getStatus(0)) {
		selectedInstrument_ = ((SetActiveView *) currentView_)->getSelectedInstrumentIndex();
		delete currentView_;
		createSetStepView();
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_ACTIVE_INDEX), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::updateInRecording() {
	bool functionButtonDown = hw_->getButtonState(buttonMap_->getMainMenuButtonIndex(MENU_FUNCTION_INDEX)) == IButtonHW::DOWN;
	bool playRecordSwitchOn = playRecordSwitch_.getStatus(1);
	if (functionButtonDown || !playRecordSwitchOn) {
		delete currentView_;
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(MENU_RECORD_INDEX), ILEDHW::OFF);
		if (functionButtonDown)  {
			for (int i = 0; i < 32; i++) {
				hw_->setLED(buttonMap_->getButtonIndex(i), ILEDHW::OFF);
			}
			createFunctionView(true);
		} else {
			createSetStepView();
		}
	}
}




void SekvojRackMainMenuView::update() {

	//Reset all counters in case play has been just pressed
	if ((currentStatus_ != FUNCTION) && (currentStatus_ != FUNCTION_FROM_RECORD)) {
		bool originalPlayValue = playRecordSwitch_.getStatus(0);
		playRecordSwitch_.update();
		bool newPlayValue = playRecordSwitch_.getStatus(0);
		if (originalPlayValue && !newPlayValue) {
			synchronizer_->reset();
			player_->resetAllInstruments();
		}
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(4), isPlaying() ? ILEDHW::ON : ILEDHW::OFF);
	}
	switch (currentStatus_) {
		case INIT:
			updateInInit();
			break;
		case ACTIVE:
			playRecordSwitch_.setStatus(1, false);
			updateInActive();
			break;
		case RECORDING:
			updateInRecording();
			break;
		case PATTERN:
			playRecordSwitch_.setStatus(1, false);
			updateInPattern();
			break;
		case FUNCTION:
		case FUNCTION_FROM_RECORD:
			playRecordSwitch_.setStatus(1, false);
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


