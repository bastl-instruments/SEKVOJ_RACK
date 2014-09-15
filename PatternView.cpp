/*
 * PatternView.cpp
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#include "PatternView.h"
#include <InstrumentDefinitions.h>
#include <BitArrayOperations.h>

PatternView::PatternView() : hw_(0), settings_(0), memory_(0), instrumentBar_(0),
							 patternSelectRadioButtons_(0),
							 currentPattern_(0) {
  currentInstrumentStatuses_ = 0;
}

PatternView::~PatternView() {
	delete patternSelectRadioButtons_;
}

void PatternView::init(IHWLayer * hw, PlayerSettings * settigns, IStepMemory * memory, InstrumentBar * instrumentBar, SekvojButtonMap * buttonMap) {
	hw_ = hw;
	settings_ = settigns;
	memory_ = memory;
	instrumentBar_ = instrumentBar;
	buttonMap_ = buttonMap;
	currentPattern_ = settings_->getCurrentPattern();

	instrumentSwitches_.init(hw_, buttonMap_->getInstrumentButtonArray(), 10);
	patternSelectRadioButtons_ = new RadioButtons(hw_, buttonMap_->getStepButtonArray(), 16);
	patternSelectRadioButtons_->setSelectedButton(currentPattern_);

	for (unsigned char i = 0; i < 16; i++) {
		hw_->setLED(buttonMap_->getStepButtonIndex(i), (i == currentPattern_)  ? IHWLayer::ON : IHWLayer::OFF);
	}
	reflectPatternChange();
}

void PatternView::reflectPatternChange() {
	settings_->setCurrentPattern(currentPattern_);
	unsigned char allDrumSettings[3];
	memory_->getPatternSettings(currentPattern_, &allDrumSettings[0]);
	currentInstrumentStatuses_ = allDrumSettings[1] << 8 | allDrumSettings[0];
	for (unsigned char i = 0; i < 10; i++) {
		bool instrumentStatus = GETBIT(currentInstrumentStatuses_, i);
		instrumentBar_->setInstrumentSelected(i, instrumentStatus);
		settings_->setInstrumentOn(InstrumentTypes::DRUM, i, instrumentStatus);
		instrumentSwitches_.setStatus(i, instrumentStatus);
	}
}

void PatternView::update() {
	patternSelectRadioButtons_->update();
	instrumentSwitches_.update();

	unsigned char newPattern = 0;
	if (patternSelectRadioButtons_->getSelectedButton(newPattern) && newPattern != currentPattern_) {
		hw_->setLED(buttonMap_->getStepButtonIndex(currentPattern_), IHWLayer::OFF);
		hw_->setLED(buttonMap_->getStepButtonIndex(newPattern), IHWLayer::ON);
		currentPattern_ = newPattern;
		reflectPatternChange();
		return;
	}
	for (unsigned char i = 0; i < 10; i++) {
		bool newStatus = instrumentSwitches_.getStatus(i);
		bool oldStatus = GETBIT(currentInstrumentStatuses_, i);
		if (newStatus != oldStatus) {
			settings_->setInstrumentOn(InstrumentTypes::DRUM, i, newStatus);
			SETBIT(currentInstrumentStatuses_, i, newStatus);
			unsigned char allDrumSettings[3];
			allDrumSettings[0] = (currentInstrumentStatuses_ << 8) >> 8;
			allDrumSettings[1] = currentInstrumentStatuses_ >> 8;
			allDrumSettings[2] = 0;
			memory_->setPatternSettings(currentPattern_, &allDrumSettings[0]);
			instrumentBar_->setInstrumentSelected(i, newStatus);
		}
	}
}
