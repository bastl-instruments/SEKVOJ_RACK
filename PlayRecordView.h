/*
 * PlayRecordView.h
 *
 *  Created on: Jul 31, 2014
 *      Author: bastl
 */

#ifndef PLAYRECORDVIEW_H_
#define PLAYRECORDVIEW_H_

#include <IView.h>
#include <IHWLayer.h>
#include <StepRecorder.h>
#include "DrumStepsView.h"
#include "SekvojButtonMap.h"


class PlayRecordView : public IView {
public:
	PlayRecordView();
	~PlayRecordView();
	void init(IHWLayer * hw, StepRecorder * recorder, SekvojButtonMap * buttonMap);
	void update();
private:
	IHWLayer * hw_;
	StepRecorder * recorder_;
	SekvojButtonMap * buttonMap_;
	unsigned int currentStatus_;
};

#endif /* PLAYRECORDVIEW_H_ */
