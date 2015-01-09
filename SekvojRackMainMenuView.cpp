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

SekvojRackMainMenuView::SekvojRackMainMenuView() : hw_(0), player_(0), recorder_(0), memory_(0), settings_(0),
							   instrumentBar_(0), buttonMap_(0), sd_(0), currentView_(0), currentViewIndex_(0),
							   currentBarIndex_(0), functionButtonDown_(false), patternButtonDown_(false),
							   currentStatus_(INIT), selectedInstrument_(0), synchronizer_(0), tapper_(0) {
}

SekvojRackMainMenuView::~SekvojRackMainMenuView() {
}

void SekvojRackMainMenuView::init(sekvojHW * hw, Player * player, StepRecorder * recorder,
						IStepMemory * memory, PlayerSettings * settings, InstrumentBar * instrumentBar,
						IButtonMap * buttonMap, StepSynchronizer * synchronizer, ITapper * tapper,
						SekvojRackSDPreset * sd) {
	hw_ = hw;
	sd_ = sd;
	tapper_ = tapper;
	player_ = player;
	recorder_ = recorder;
	memory_ = memory;
	settings_ = settings;
	instrumentBar_ = instrumentBar;
	buttonMap_ = buttonMap;
	synchronizer_ = synchronizer;

	createSetStepView();

	for (unsigned char index = 0; index < 6; index ++) {
		hw_->setLED(buttonMap_->getMainMenuButtonIndex(index), ILEDHW::OFF);
	}

	unsigned char * recordButton = buttonMap_->getMainMenuButtonArray() + 2;
	unsigned char * activeButton = buttonMap_->getMainMenuButtonArray() + 1;

	playRecordSwitch_.init(hw_, recordButton, 2, IButtonHW::DOWN);
	playRecordSwitch_.setStatus(0, true);
	activeSwitch_.init(hw_, activeButton, 1, IButtonHW::DOWN);
}

void SekvojRackMainMenuView::createSetStepView() {
	SetStepView * setStepView = new SetStepView();
	setStepView->init(hw_, memory_, player_, instrumentBar_, buttonMap_,
					  settings_->getCurrentPattern(), 6, selectedInstrument_, settings_, currentBarIndex_);
	currentView_ = (IView*)setStepView;
	currentStatus_ = INIT;
}

void SekvojRackMainMenuView::createFunctionView(bool fromRecord) {
	currentStatus_ = fromRecord ?  FUNCTION_FROM_RECORD : FUNCTION;
	SettingsAndFunctionsView * functionView = new SettingsAndFunctionsView();
	functionView->init(hw_, settings_,instrumentBar_, buttonMap_, memory_, selectedInstrument_,
					   currentBarIndex_, player_, tapper_, sd_);
	currentView_ = (IView*)functionView;
	hw_->setLED(buttonMap_->getFunctionButtonIndex(), ILEDHW::ON);
}

void SekvojRackMainMenuView::createPatternView(bool fromRecord, bool fromActive) {
	currentStatus_ = PATTERN;
	if (fromRecord || fromActive) {
		currentStatus_ = fromRecord ? PATTERN_FROM_RECORD : PATTERN_FROM_ACTIVE;
	}
	clearAllDiods();
	PatternView * patternView = new PatternView();
	patternView->init(hw_, settings_, memory_, instrumentBar_, buttonMap_);
	currentView_ = (IView*)patternView;
	hw_->setLED(buttonMap_->getPatternButtonIndex(), ILEDHW::ON);

}

void SekvojRackMainMenuView::createRecordView() {
	currentStatus_ = RECORDING;
	PlayRecordView * playRecordView = new PlayRecordView();
	playRecordView->init(hw_, recorder_, buttonMap_, synchronizer_);
	currentView_ = (IView*)playRecordView;
	instrumentBar_->resetSelected();
	playRecordSwitch_.setStatus(1, true);
	hw_->setLED(buttonMap_->getRecordButtonIndex(), ILEDHW::ON);
}

void SekvojRackMainMenuView::createActiveView() {
	currentStatus_ = ACTIVE;
	SetActiveView * activeView = new SetActiveView();
	activeView->init(hw_, memory_, player_, instrumentBar_, buttonMap_, selectedInstrument_);
	currentView_ = (IView*)activeView;
	hw_->setLED(buttonMap_->getActiveButtonIndex(), ILEDHW::ON);

}

inline void SekvojRackMainMenuView::updateInInit() {
	activeSwitch_.update();
	((SetStepView*)currentView_)->setPlaying(playRecordSwitch_.getStatus(0));
	if (playRecordSwitch_.getStatus(1)) {
		destroyInitView();
		createRecordView();
		return;
	}
	if (activeSwitch_.getStatus(0)) {
		destroyInitView();
		createActiveView();
		return;
	}
	if (patternButtonDown_) {
		destroyInitView();
		createPatternView(false, false);
		return;
	}
	if (functionButtonDown_) {
		destroyInitView();
		createFunctionView(false);
		return;
	}
}

void SekvojRackMainMenuView::destroyInitView() {
	selectedInstrument_ = ((SetStepView *) currentView_)->getSelectedIndstrumentIndex();
	currentBarIndex_ = ((SetStepView *) currentView_)->getSelectedBarIndex();
	delete currentView_;
	clearAllDiods();
}

inline void SekvojRackMainMenuView::updateInPattern() {
	if (!patternButtonDown_) {
		delete currentView_;
		if (currentStatus_ == PATTERN_FROM_RECORD) {
			createRecordView();
		} else if (currentStatus_ == PATTERN_FROM_ACTIVE) {
			createActiveView();
		} else {
			createSetStepView();
		}
		hw_->setLED(buttonMap_->getPatternButtonIndex(), ILEDHW::OFF);
	}
}

inline void SekvojRackMainMenuView::updateInFunction() {
	if (!functionButtonDown_) {
		delete currentView_;
		clearAllDiods();
		if (currentStatus_ == FUNCTION_FROM_RECORD) {
			createRecordView();
		} else {
			createSetStepView();
		}
		hw_->setLED(buttonMap_->getFunctionButtonIndex(), ILEDHW::OFF);
	}
}

inline void SekvojRackMainMenuView::updateInActive() {
	activeSwitch_.update();
	if (patternButtonDown_ || !activeSwitch_.getStatus(0)) {
		selectedInstrument_ = ((SetActiveView *) currentView_)->getSelectedInstrumentIndex();
		delete currentView_;
		if (patternButtonDown_) {
			createPatternView(false, true);
		} else {
			createSetStepView();
		}
		hw_->setLED(buttonMap_->getActiveButtonIndex(), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::clearAllDiods() {
	for (int i = 0; i < 32; i++) {
		hw_->setLED(buttonMap_->getButtonIndex(i), ILEDHW::OFF);
	}
}

inline void SekvojRackMainMenuView::updateInRecording() {
	bool playRecordSwitchOn = playRecordSwitch_.getStatus(1);
	if (functionButtonDown_ || patternButtonDown_ || !playRecordSwitchOn ) {
		delete currentView_;
		hw_->setLED(buttonMap_->getRecordButtonIndex(), ILEDHW::OFF);
		if (functionButtonDown_)  {
			clearAllDiods();
			createFunctionView(true);
		} else if (patternButtonDown_) {
			clearAllDiods();
			createPatternView(true, false);
		} else {
			createSetStepView();
		}
	}
}

void SekvojRackMainMenuView::update() {

	functionButtonDown_ = hw_->getButtonState(buttonMap_->getFunctionButtonIndex()) == IButtonHW::DOWN;
	patternButtonDown_ = hw_->getButtonState(buttonMap_->getPatternButtonIndex()) == IButtonHW::DOWN;

	//Reset all counters in case play has been just pressed
	if ((currentStatus_ != FUNCTION) && (currentStatus_ != FUNCTION_FROM_RECORD)) {
		bool originalPlayValue = playRecordSwitch_.getStatus(0);
		playRecordSwitch_.update();
		bool newPlayValue = playRecordSwitch_.getStatus(0);
		if (originalPlayValue && !newPlayValue) {
			synchronizer_->reset();
			player_->resetAllInstruments();
		}
		hw_->setLED(buttonMap_->getPlayButtonIndex(), isPlaying() ? ILEDHW::ON : ILEDHW::OFF);
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
		case PATTERN_FROM_RECORD:
			playRecordSwitch_.setStatus(1, false);
		case PATTERN:
		case PATTERN_FROM_ACTIVE:
			updateInPattern();
			break;
		case FUNCTION_FROM_RECORD:
			playRecordSwitch_.setStatus(1, false);
		case FUNCTION:
			updateInFunction();
			break;
	}
	if (currentView_) {
		currentView_->update();
	}
}


