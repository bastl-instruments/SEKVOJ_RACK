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
#include <FunctionViewExtra.h>
#include <IButtonHW.h>
#include <SekvojModulePool.h>

SekvojRackMainMenuView::SekvojRackMainMenuView() : currentView_(0), currentViewIndex_(0), currentBarIndex_(0),
												   functionButtonDown_(false), patternButtonDown_(false), patternButtonMomentary_(false),
												   currentStatus_(INIT), selectedInstrument_(0) {
}

void SekvojRackMainMenuView::init(void (*playerStatusChangedCallback)()) {

	clearAllDiods();
	createSetStepView(false);

	playerStatusChangedCallback_ = playerStatusChangedCallback;
	isPlaying_ = true;
	unsigned char * activeButton = SekvojModulePool::buttonMap_->getMainMenuButtonArray() + 1;
	activePlayRecordSwitch_.init(SekvojModulePool::hw_, activeButton, 4, true, IButtonHW::DOWN);
	activePlayRecordSwitch_.setStatus(1, isPlaying_);
}

void SekvojRackMainMenuView::createSetStepView(bool fromPattern) {
	SetStepView * setStepView = new SetStepView();
	setStepView->init(SekvojModulePool::settings_->getCurrentPattern(), 6,
					  selectedInstrument_, currentBarIndex_);
	currentView_ = (IView*)setStepView;
	currentStatus_ = fromPattern ? INIT_FROM_PATTERN : INIT;
}

void SekvojRackMainMenuView::createFunctionView(UIStatus viewSourceDefinition) {
	currentStatus_ = viewSourceDefinition;
	clearBottomPartDiods();
	SettingsAndFunctionsView * functionView = new SettingsAndFunctionsView();
	functionView->init(selectedInstrument_, currentBarIndex_);
	currentView_ = (IView*)functionView;
	activePlayRecordSwitch_.setStatus(3, patternButtonMomentary_);
}

void SekvojRackMainMenuView::createPatternView(UIStatus viewSourceDefinition) {
	currentStatus_ = viewSourceDefinition;
	clearBottomPartDiods();
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
	//ToPatternView
	bool switchToPatternView = patternButtonMomentary_ ?
			activePlayRecordSwitch_.getStatus(3) : patternButtonDown_;
	if (switchToPatternView) {
		destroyInitView();
		createPatternView(PATTERN);
		return;
	}
	if (functionButtonDown_) {
		destroyInitView();
		createFunctionView(FUNCTION);
		return;
	}
}

void SekvojRackMainMenuView::destroyInitView() {
	selectedInstrument_ = ((SetStepView *) currentView_)->getSelectedIndstrumentIndex();
	currentBarIndex_ = ((SetStepView *) currentView_)->getSelectedBarIndex();
	delete currentView_;
}

inline void SekvojRackMainMenuView::updateInPattern() {
	if (patternButtonMomentary_) {
		activePlayRecordSwitch_.setStatus(0, false);
	}
	activePlayRecordSwitch_.setStatus(2, false);
	bool modeOn = patternButtonMomentary_ ?
			activePlayRecordSwitch_.getStatus(3) :
			patternButtonDown_;
	if (!modeOn) {
		delete currentView_;
		if (!patternButtonMomentary_) {
			activePlayRecordSwitch_.setStatus(3, false);
		}
		if (currentStatus_ == PATTERN_FROM_RECORD) {
			createRecordView();
		} else if (currentStatus_ == PATTERN_FROM_ACTIVE) {
			createActiveView();
		} else {
			createSetStepView(false);
		}
	} else if (jumpButtonDown_ && patternButtonMomentary_) {
		delete currentView_;
		createSetStepView(true);
	} else if (functionButtonDown_ && patternButtonMomentary_) {
		delete currentView_;
		createFunctionView(FUNCTION_FROM_PATTERN);
	}

}

inline void SekvojRackMainMenuView::updateInFunction() {
	SekvojModulePool::settings_->setPatternMomentary(activePlayRecordSwitch_.getStatus(3));
	if (!functionButtonDown_) {
		activePlayRecordSwitch_.setStatus(3, false);
		delete currentView_;
		SekvojModulePool::instrumentBar_->setActive(true);
		activePlayRecordSwitch_.setStatus(0, false);
		activePlayRecordSwitch_.setStatus(1, isPlaying_);
		if (currentStatus_ == FUNCTION_FROM_RECORD) {
			activePlayRecordSwitch_.setStatus(2, true);
			createRecordView();
		} else if (currentStatus_ == FUNCTION_FROM_PATTERN) {
			activePlayRecordSwitch_.setStatus(3, true);
			createPatternView(PATTERN);
		} else {
			activePlayRecordSwitch_.setStatus(2, false);
			createSetStepView(false);
		}
	} else if (activeButtonDown_) {
		if (!functionInActive_) {
			delete currentView_;
			clearBottomPartDiods();
			FunctionViewExtra * functionView = new FunctionViewExtra();
			functionView->init();
			currentView_ = (IView*)functionView;
			functionInActive_ = true;
		}
	} else {
		if (functionInActive_) {
			delete currentView_;
			activePlayRecordSwitch_.setStatus(0, false);
			createFunctionView(currentStatus_);
			functionInActive_ = false;
		}
	}
}

inline void SekvojRackMainMenuView::updateInActive() {
	activePlayRecordSwitch_.setStatus(2, false);
	if (patternButtonMomentary_) {
		activePlayRecordSwitch_.setStatus(3, false);
	}
	bool switchToPattern = (patternButtonDown_ && !patternButtonMomentary_);
	if (switchToPattern || !activePlayRecordSwitch_.getStatus(0)) {
		selectedInstrument_ = ((SetActiveView *) currentView_)->getSelectedInstrumentIndex();
		currentBarIndex_ = ((SetActiveView *) currentView_)->getSelectedBarIndex();
		delete currentView_;
		if (switchToPattern) {
			createPatternView(PATTERN_FROM_ACTIVE);
		} else {
			createSetStepView(false);
		}
	}
}

void SekvojRackMainMenuView::clearAllDiods() {
	for (int i = 0; i < 32; i++) {
		SekvojModulePool::setLED(SekvojModulePool::buttonMap_->getButtonIndex(i), ILEDHW::OFF);
	}
}

void SekvojRackMainMenuView::clearBottomPartDiods() {
	for (unsigned char button = 0; button < 20; button++) {
		unsigned char index = button < 16 ? SekvojModulePool::buttonMap_->getStepButtonIndex(button) :
											SekvojModulePool::buttonMap_->getSubStepButtonIndex(button - 16);
		SekvojModulePool::setLED(index, ILEDHW::OFF);
	}
}

inline void SekvojRackMainMenuView::updateInRecording() {
	activePlayRecordSwitch_.setStatus(0, false);
	bool playRecordSwitchOn = activePlayRecordSwitch_.getStatus(2);
	bool switchToPattern = patternButtonDown_ && ! patternButtonMomentary_;
	if (functionButtonDown_ || switchToPattern || !playRecordSwitchOn ) {
		delete currentView_;
		if (functionButtonDown_)  {
			createFunctionView(FUNCTION_FROM_RECORD);
		} else if (switchToPattern) {
			createPatternView(PATTERN_FROM_RECORD);
		} else {
			createSetStepView(false);
		}
	}
}

inline void SekvojRackMainMenuView::updateInJumpInit() {
	((SetStepView*)currentView_)->setPlaying(activePlayRecordSwitch_.getStatus(1));
	if (!jumpButtonDown_) {
		destroyInitView();
		createPatternView(PATTERN);
	}
}

void SekvojRackMainMenuView::update() {

	functionButtonDown_ = SekvojModulePool::hw_->isButtonDown(SekvojModulePool::buttonMap_->getFunctionButtonIndex());
	patternButtonDown_ = SekvojModulePool::hw_->isButtonDown(SekvojModulePool::buttonMap_->getPatternButtonIndex());
	activeButtonDown_ = SekvojModulePool::hw_->isButtonDown(SekvojModulePool::buttonMap_->getActiveButtonIndex());
	jumpButtonDown_ = SekvojModulePool::hw_->isButtonDown(SekvojModulePool::buttonMap_->getJumpButtonIndex());
	patternButtonMomentary_ = SekvojModulePool::settings_->isPatternMomentary();
	SekvojModulePool::setLED(SekvojModulePool::buttonMap_->getFunctionButtonIndex(), functionButtonDown_ ? ILEDHW::ON : ILEDHW::OFF);

	//Reset all counters in case play has been just pressed
	bool originalPlayValue = activePlayRecordSwitch_.getStatus(1);
	activePlayRecordSwitch_.update();
	bool newPlayValue = activePlayRecordSwitch_.getStatus(1);
	if ((currentStatus_ != FUNCTION) && (currentStatus_ != FUNCTION_FROM_RECORD)) {
		if (originalPlayValue && !newPlayValue) {
			playerStatusChangedCallback_();
		}
		isPlaying_ = newPlayValue;
		if (originalPlayValue != newPlayValue && SekvojModulePool::settings_->getPlayerMode() == PlayerSettings::MASTER) {
			SekvojModulePool::hw_->setTrigger(7, true, 20);
		}
	}
	ILEDHW::LedState offState = ILEDHW::OFF;
	switch (currentStatus_) {
		case INIT_FROM_PATTERN:
			updateInJumpInit();
			break;
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
		case FUNCTION_FROM_PATTERN:
		case FUNCTION:
			updateInFunction();
			if (SekvojModulePool::settings_->getPlayerMode() == PlayerSettings::MASTER) {
				offState = ILEDHW::DULLON;
			}
			break;
	}
	if (currentView_) {
		currentView_->update();
	}
	SekvojModulePool::setLED(SekvojModulePool::buttonMap_->getPlayButtonIndex(), SekvojModulePool::synchronizer_->getCurrentStepNumber() % 16 == 0 && isPlaying_ ? ILEDHW::ON : offState);
}


