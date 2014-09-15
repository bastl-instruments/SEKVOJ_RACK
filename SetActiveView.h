
/*
 * SetActiveView.h
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#ifndef SETACTIVEVIEW_H_
#define SETACTIVEVIEW_H_

#include <IView.h>
#include <IHWLayer.h>
#include <IStepMemory.h>
#include <RadioButtons.h>
#include <Switches.h>
#include <Player.h>
#include "InstrumentBar.h"
#include "SekvojButtonMap.h"

class SetActiveView : public IView {
public:
	SetActiveView();
	~SetActiveView();
	void init(	IHWLayer * hw, IStepMemory * memory, Player * player, InstrumentBar * instrumentBar,
				SekvojButtonMap * buttonMap, unsigned char pattern);
	void update();
	void updateActives();
	void updateConfiguration();
private:
	IHWLayer * hw_;
	IStepMemory * memory_;
	Player * player_;
	InstrumentBar * instrumentBar_;
	SekvojButtonMap * buttonMap_;


	unsigned char currentPattern_;
	unsigned char currentPanIndex_;
	unsigned char currentInstrumentIndex_;

	unsigned int currentStatuses_;

	RadioButtons * panButtons_;
	RadioButtons * instrumentButtons_;

};

#endif /* SETACTIVEVIEW_H_ */
