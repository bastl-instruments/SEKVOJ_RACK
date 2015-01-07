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

#define MENU_PLAY_INDEX 4
#define MENU_RECORD_INDEX 5
#define MENU_TAP_INDEX 0
#define MENU_FUNCTION_INDEX 3
#define MENU_PATTERN_INDEX 2
#define MENU_ACTIVE_INDEX 1

class SekvojRackMainMenuView : public IView {
public:
	SekvojRackMainMenuView();
	~SekvojRackMainMenuView();
	void init(sekvojHW * hw, Player * player, StepRecorder * recorder,
			  IStepMemory * memory, PlayerSettings * settings, InstrumentBar * instrumentBar,
			  IButtonMap * buttonMap, StepSynchronizer * synchronizer, ITapper * tapper);
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
	Switches playRecordSwitch_;
	Switches activeSwitch_;

	IView * currentView_;
	unsigned char currentViewIndex_;
	unsigned char currentPattern_;

	bool functionButtonDown_;
	bool patternButtonDown_;

	UIStatus currentStatus_;
	unsigned char selectedInstrument_;

	StepSynchronizer * synchronizer_;
	ITapper * tapper_;

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

};

inline bool SekvojRackMainMenuView::isPlaying() {
	return playRecordSwitch_.getStatus(0);
}

#endif /* SEKVOJRACKMAINMENUVIEW_H_ */
