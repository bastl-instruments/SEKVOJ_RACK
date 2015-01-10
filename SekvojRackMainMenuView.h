/*
 * SekvojRackMainMenuView.h
 *
 *  Created on: Jul 25, 2014
 *      Author: bastl
 */

#ifndef SEKVOJRACKMAINMENUVIEW_H_
#define SEKVOJRACKMAINMENUVIEW_H_

#include <IView.h>
#include "sekvojHW.h"
#include <Player.h>
#include <IStepMemory.h>
#include <RadioButtons.h>
#include <PlayerSettings.h>
#include <BastlMetronome.h>
#include <StepRecorder.h>
#include "InstrumentBar.h"
#include <IButtonMap.h>
#include <Switches.h>
#include <StepSynchronizer.h>
#include <ITapper.h>
#include <SekvojRackSDPreset.h>

class SekvojRackMainMenuView : public IView {
public:
	SekvojRackMainMenuView();
	~SekvojRackMainMenuView();
	void init(sekvojHW * hw, Player * player, StepRecorder * recorder,
			  IStepMemory * memory, PlayerSettings * settings, InstrumentBar * instrumentBar,
			  IButtonMap * buttonMap, StepSynchronizer * synchronizer, ITapper * tapper,
			  SekvojRackSDPreset * sd);
	void update();
	bool isPlaying();
private:

	enum UIStatus{INIT, ACTIVE, RECORDING, PATTERN, PATTERN_FROM_ACTIVE, PATTERN_FROM_RECORD, FUNCTION, FUNCTION_FROM_RECORD};

	sekvojHW * hw_;
	Player * player_;
	StepRecorder * recorder_;
	IStepMemory * memory_;
	PlayerSettings * settings_;
	InstrumentBar * instrumentBar_;
	IButtonMap * buttonMap_;
	SekvojRackSDPreset * sd_;
	Switches playRecordSwitch_;
	Switches activeSwitch_;

	IView * currentView_;
	unsigned char currentViewIndex_;
	unsigned char currentBarIndex_;

	bool functionButtonDown_;
	bool patternButtonDown_;

	UIStatus currentStatus_;
	unsigned char selectedInstrument_;

	StepSynchronizer * synchronizer_;
	ITapper * tapper_;
	bool isPlaying_;

	void updateInInit();
	void updateInActive();
	void updateInRecording();
	void updateInPattern();
	void updateInFunction();
	void createSetStepView();
	void destroyInitView();
	void createFunctionView(bool fromRecord);
	void createRecordView();
	void createPatternView(bool fromRecord, bool fromActive);
	void createActiveView();
	void clearAllDiods();
	void clearBottomPartDiods();

};

inline bool SekvojRackMainMenuView::isPlaying() {
	return isPlaying_;
}

#endif /* SEKVOJRACKMAINMENUVIEW_H_ */
