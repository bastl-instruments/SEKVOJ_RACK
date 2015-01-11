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
#include <SekvojModulePool.h>

SekvojRackMainMenuView::SekvojRackMainMenuView() : currentView_(0), currentViewIndex_(0), currentBarIndex_(0),
												   functionButtonDown_(false), patternButtonDown_(false),
												   currentStatus_(INIT), selectedInstrument_(0) {
}

SekvojRackMainMenuView::~SekvojRackMainMenuView() {
}

void SekvojRackMainMenuView::init() {

	clearAllDiods();
	createSetStepView();

	isPlaying_ = true;
	unsigned char * activeButton = SekvojModulePool::buttonMap_->getMainMenuButtonArray() + 1;
	activePlayRecordSwitch_.init(SekvojModulePool::hw_, activeButton, 3, true, IButtonHW::DOWN);
	activePlayRecordSwitch_.setStatus(1, isPlaying_);
}

void SekvojRackMainMenuView::createSetStepView() {
	SetStepView * setStepView = new SetStepView();
	setStepView->init(SekvojModulePool::settings_->getCurrentPattern(), 6,
					  selectedInstrument_, currentBarIndex_);
	currentView_ = (IView*)setStepView;
	currentStatus_ = INIT;
}

void SekvojRackMainMenuView::createFunctionView(bool fromRecord) {
	currentStatus_ = fromRecord ?  FUNCTION_FROM_RECORD : FUNCTION;
	clearBottomPartDiods();
	SettingsAndFunctionsView * functionView = new SettingsAndFunctionsView();
	functionView->init(selectedInstrument_, currentBarIndex_);
	currentView_ = (IView*)functionView;
}

void SekvojRackMainMenuView::createPatternView(bool fromRecord, bool fromActive) {
	currentStatus_ = PATTERN;
	clearBottomPartDiods();
	if (fromRecord || fromActive) {
		currentStatus_ = fromRecord ? PATTERN_FROM_RECORD : PATTERN_FROM_ACTIVE;
	}
	PatternView * patternView = new PatternView();
	patternView->init();
	currentView_ = (IView*)patternView;
}

void SekvojRackMainMenuView::createRecordView() {
	currentStatus_ = RECORDING;
	clearBottomPartDiods();
	PlayRecordView * playRecordView = new PlayRecordView();
	currentView_ = (IView*)playRecordView;
	SekvojModulePool::instrumentBar_->resetSelected();
}

void SekvojRackMainMenuView::createActiveView() {
	currentStatus_ = ACTIVE;
	SetActiveView * activeView = new SetActiveView();
	activeView->init(selectedInstrument_, currentBarIndex_);
	currentView_ = (IView*)activeView;
}

inline void SekvojRackMainMenuView::updateInInit() {
	((SetStepView*)currentView_)->setPlaying(activePlayRecordSwitch_.getStatus(1));
	if (activePlayRecordSwitch_.getStatus(2)) {
		destroyInitView();
		createRecordView();
		return;
	}
	if (activePlayRecordSwitch_.getStatus(0)) {
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
	}
}

inline void SekvojRackMainMenuView::updateInFunction() {
	if (!functionButtonDown_) {
		delete currentView_;
		activePlayRecordSwitch_.setStatus(1, isPlaying_);
		if (currentStatus_ == FUNCTION_FROM_RECORD) {
			activePlayRecordSwitch_.setStatus(2, true);
			createRecordView();
		} else {
			activePlayRecordSwitch_.setStatus(2, false);
			createSetStepView();
		}
	}
}

inline void SekvojRackMainMenuView::updateInActive() {
	if (patternButtonDown_ || !activePlayRecordSwitch_.getStatus(0)) {
		selectedInstrument_ = ((SetActiveView *) currentView_)->getSelectedInstrumentIndex();
		currentBarIndex_ = ((SetActiveView *) currentView_)->getSelectedBarIndex();
		delete currentView_;
		if (patternButtonDown_) {
			createPatternView(false, true);
		} else {
			createSetStepView();
		}
	}
}

void SekvojRackMainMenuView::clearAllDiods() {
	for (int i = 0; i < 32; i++) {
		SekvojModulePool::hw_->setLED(SekvojModulePool::buttonMap_->getButtonIndex(i), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::clearBottomPartDiods() {
	for (unsigned char button = 0; button < 20; button++) {
		unsigned char index = button < 16 ? SekvojModulePool::buttonMap_->getStepButtonIndex(button) :
											SekvojModulePool::buttonMap_->getSubStepButtonIndex(button - 16);
		SekvojModulePool::hw_->setLED(index, ILEDHW::OFF);
	}
}

inline void SekvojRackMainMenuView::updateInRecording() {
	bool playRecordSwitchOn = activePlayRecordSwitch_.getStatus(2);
	if (functionButtonDown_ || patternButtonDown_ || !playRecordSwitchOn ) {
		delete currentView_;
		if (functionButtonDown_)  {
			createFunctionView(true);
		} else if (patternButtonDown_) {
			createPatternView(true, false);
		} else {
			createSetStepView();
		}
	}
}

void SekvojRackMainMenuView::update() {

	functionButtonDown_ = SekvojModulePool::hw_->isButtonDown(SekvojModulePool::buttonMap_->getFunctionButtonIndex());
	patternButtonDown_ = SekvojModulePool::hw_->isButtonDown(SekvojModulePool::buttonMap_->getPatternButtonIndex());
	SekvojModulePool::hw_->setLED(SekvojModulePool::buttonMap_->getFunctionButtonIndex(), functionButtonDown_ ? ILEDHW::ON : ILEDHW::OFF);
	SekvojModulePool::hw_->setLED(SekvojModulePool::buttonMap_->getPatternButtonIndex(), patternButtonDown_ ? ILEDHW::ON : ILEDHW::OFF);

	//Reset all counters in case play has been just pressed
	bool originalPlayValue = activePlayRecordSwitch_.getStatus(1);
	activePlayRecordSwitch_.update();
	bool newPlayValue = activePlayRecordSwitch_.getStatus(1);
	if ((currentStatus_ != FUNCTION) && (currentStatus_ != FUNCTION_FROM_RECORD)) {
		if (originalPlayValue && !newPlayValue) {
			SekvojModulePool::synchronizer_->reset();
			SekvojModulePool::player_->resetAllInstruments();
		}
		isPlaying_ = newPlayValue;
	}
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
		case PATTERN_FROM_RECORD:
		case PATTERN:
		case PATTERN_FROM_ACTIVE:
			updateInPattern();
			break;
		case FUNCTION_FROM_RECORD:
		case FUNCTION:
			updateInFunction();
			break;
	}
	if (currentView_) {
		currentView_->update();
	}
}


