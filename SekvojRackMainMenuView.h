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
#include <SekvojModulePool.h>

class SekvojRackMainMenuView : public IView {
public:
	SekvojRackMainMenuView();
	void init(void (*playerStatusChangedCallback)());
	void update();
	bool isPlaying();
private:

	enum UIStatus{INIT, INIT_FROM_PATTERN, ACTIVE, RECORDING, PATTERN, PATTERN_FROM_ACTIVE, FUNCTION_FROM_PATTERN, FUNCTION, FUNCTION_FROM_RECORD};

	Switches activePlayRecordSwitch_;
	IView * currentView_;
	unsigned char currentViewIndex_;
	unsigned char currentBarIndex_;
	bool functionButtonDown_;
	bool patternButtonMomentary_;
	bool functionInActive_;
	bool activeButtonDown_;
	bool patternButtonDown_;
	bool jumpButtonDown_;

	UIStatus currentStatus_;
	unsigned char selectedInstrument_;
	bool isPlaying_;
	void (*playerStatusChangedCallback_)();

	void updateInInit();
	void updateInJumpInit();
	void updateInActive();
	void updateInRecording();
	void updateInPattern();
	void updateInFunction();
	void createSetStepView(bool fromPattern);
	void destroyInitView();
	void createFunctionView(UIStatus viewSourceDefinition);
	void createRecordView();
	void createPatternView(UIStatus viewSourceDefinition);
	void createActiveView();
	void clearAllDiods();
	void clearBottomPartDiods();

};

inline bool SekvojRackMainMenuView::isPlaying() {
	return isPlaying_;
}

#endif /* SEKVOJRACKMAINMENUVIEW_H_ */
