/*
 * PlayRecordView.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: bastl
 */

#include "PlayRecordView.h"
#include <MIDICommand.h>
#include <DrumStep.h>

PlayRecordView::PlayRecordView() : hw_(0), recorder_(0), buttonMap_(0) {
}

PlayRecordView::~PlayRecordView() {
}

void PlayRecordView::init(IHWLayer * hw, StepRecorder * recorder, SekvojButtonMap * buttonMap) {
	hw_ = hw;
	recorder_ = recorder;
	buttonMap_ = buttonMap;
}

void PlayRecordView::update() {
	unsigned int newStatuses = 0;
	for (unsigned char i = 0; i < 10; i++) {
		bool currentStatus = (currentStatus_ & (1 << i)) != 0;
		bool newStatus = hw_->getButtonState(buttonMap_->getInstrumentButtonIndex(i)) == IHWLayer::DOWN;
		if (!currentStatus && newStatus) {
			recorder_->recordNote(i);
		}
		if (newStatus) {
			newStatuses = newStatuses | 1 << i;
		} else {
			newStatuses = newStatuses & ~(1 << i);
		}
	}
	currentStatus_ = newStatuses;

}
