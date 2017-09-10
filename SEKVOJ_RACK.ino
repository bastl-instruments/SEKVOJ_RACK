
#ifdef EXTERNAL_IDE

#include <Arduino.h>

int main(void) {

  init();
  setup();

  while(true) {
    loop();
  }
}

#endif

#include <SekvojModulePool.h>
#include <portManipulations.h>
#include "sekvojHW.h"
#include <Player.h>
#include <StepGenerator.h>
#include <BPMConverter.h>
#include <ArduinoMIDICommandProcessor.h>
#include <PlayerSettings.h>
#include <NoVelocityStepMemory.h>
#include <RackInstrumentDefinitions.h>
#include <StepSwinger.h>
//#include <MIDI.h>


#include "SekvojRackMainMenuView.h"
#include "SekvojRackButtonMap.h"
#include <SekvojRackSDPreset.h>
#include <InstrumentBar.h>
#include <StepRecorder.h>
#include <StepSynchronizer.h>
#include <StepMultiplier.h>
#include <EEPROM.h>
#include <SimplifiedTapper.h>

Player * player;
ArduinoMIDICommandProcessor * processor;
NoVelocityStepMemory memory;
PlayerSettings * settings;
SekvojRackMainMenuView mainMenu;
BastlStepper * stepper;
StepRecorder recorder;
InstrumentBar instrumentBar;
SekvojRackButtonMap buttonMap;
StepSynchronizer synchronizer;
SekvojRackSDPreset sdpreset;
SimplifiedTapper tapper;
StepSwinger swinger;

unsigned char localStep = 0;

extern sekvojHW hardware;
unsigned int bastlCyclesPerSecond = 0;
bool slave = false;
unsigned char memoryData[290];

void stepperStep() {
	if (mainMenu.isPlaying()) {
		player->stepFourth();
		synchronizer.doStep();
		recorder.update();
		if (synchronizer.getCurrentStepNumber() % 4 == 0) {
			hardware.setTrigger(6, true,10);// ILEDsAndButtonsHW::TRIGGER_ON);
		}
	}
}

void instrumentEvent(unsigned char instrumentId, DrumStep::DrumVelocityType velocity, bool isOn) {
	instrumentBar.setInstrumentPlaying(instrumentId, isOn);
	if (isOn) {
		if (settings->getDrumInstrumentEventType(instrumentId) == PlayerSettings::GATE) {
			hardware.setTrigger(instrumentId, true, false);//ILEDsAndButtonsHW::GATE_ON);
		} else {
			hardware.setTrigger(instrumentId, true, true);//ILEDsAndButtonsHW::TRIGGER_ON);
		}
	} else if (settings->getDrumInstrumentEventType(instrumentId) == PlayerSettings::GATE){
		hardware.setTrigger(instrumentId, false, false);//ILEDsAndButtonsHW::GATE_OFF);
	}
}

void clockInCall() {
	if (settings->getPlayerMode() == PlayerSettings::SLAVE && mainMenu.isPlaying()) {
		stepper->doStep(hardware.getElapsedBastlCycles());
	}
}

void rstInCall() {
	player->resetAllInstruments();
	synchronizer.reset();
	stepper->reset();
}

void tapStep() {
	if (settings->getPlayerMode() == PlayerSettings::MASTER) {
		if (tapper.anyStepDetected()) {
			unsigned char timeUnitsPerStep = tapper.getTimeUnitsPerStep();
			((StepGenerator *)stepper)->setTimeUnitsPerStep(timeUnitsPerStep);
			settings->setBPM(BPMConverter::timeUnitsToBPM(timeUnitsPerStep, bastlCyclesPerSecond), false);
		}
		stepper->doStep(hardware.getElapsedBastlCycles());
	}
}

void patternChanged(unsigned char originalPattenrIndex, unsigned char newPatternIndex) {
	sdpreset.setPatternData(originalPattenrIndex);
	sdpreset.getPatternData(newPatternIndex);

}

unsigned char getMultiplicationFromEnum(PlayerSettings::MultiplicationType type) {
	unsigned char multiplication = 1;
	for (unsigned char i = 0; i < (char)type; i++) {
		multiplication *= 2;
	}
	return multiplication;
}

void multiplicationChanged(PlayerSettings::MultiplicationType type) {
	if (settings->getPlayerMode() == PlayerSettings::SLAVE) {
		((StepMultiplier *)stepper)->setMultiplication(getMultiplicationFromEnum(type));
	}
}

void bpmChanged(unsigned int bpm) {
	if (settings->getPlayerMode() == PlayerSettings::MASTER) {
		((StepGenerator *)stepper)->setTimeUnitsPerStep(BPMConverter::bpmToTimeUnits(bpm, bastlCyclesPerSecond));
	}
}

BastlStepper * createBastlStepper(PlayerSettings::PlayerMode mode) {
	 if (mode == PlayerSettings::MASTER) {
	    StepGenerator * generator = new StepGenerator(&stepperStep, &swinger);
	    //generator->init(&stepperStep, BPMConverter::bpmToTimeUnits(settings->getBPM(), hardware.getBastlCyclesPerSecond()));
	    generator->setTimeUnitsPerStep(BPMConverter::bpmToTimeUnits(settings->getBPM(), bastlCyclesPerSecond));
	    return generator;
	  } else {
	    StepMultiplier * multiplier = new StepMultiplier(&stepperStep, &swinger, bastlCyclesPerSecond);
	    multiplier->setMultiplication(getMultiplicationFromEnum(settings->getMultiplication()));
	    return multiplier;
	  }
}

void playerModeChanged(PlayerSettings::PlayerMode mode) {
	player->resetAllInstruments();
	synchronizer.reset();
	if (stepper) {
		delete stepper;
	}
	stepper = createBastlStepper(mode);
	hardware.setResetState(mode == PlayerSettings::MASTER);
}

void settingsChanged() {
	unsigned char data[11];
	settings->getInByteArray(data);
	sdpreset.setSettingsData(data);
	hardware.setMutes(settings->getInstrumentMuteByte());
	hardware.setTriggerLength(settings->getTriggerLength());
	instrumentBar.setInstrumentsMutes(settings->getInstrumentMuteByte());
	swinger.setSwing(settings->getSwing());

}

void setup() {

	swinger.init(& synchronizer);
	swinger.setSwing(80);

	bastlCyclesPerSecond = hardware.getBastlCyclesPerSecond();
	instrumentBar.init(&hardware, &buttonMap, 6);

	settings = new PlayerSettings();
	settings->setPatternChangedCallback(&patternChanged);
	settings->setMultiplicationChangedCallback(&multiplicationChanged);
	settings->setBPMChangedCallback(&bpmChanged);
	settings->setPlayerModeChangedCallback(&playerModeChanged);
	settings->setSettingsChangedCallback(&settingsChanged);

	//Here the pointer to the SD Card memory shall be set.
	memory.setDataReference(memoryData);
	memory.makeAllInstrumentsActiveUpTo(15);
	memory.clearStepsForAllInstruments();

	unsigned char data[11];
	settings->getInByteArray(data);
	sdpreset.initCard(memoryData, data);
	sdpreset.getSettingsData(data);
	//sdpreset.setSettingsData(data);
	settings->loadFromByteArray(data);
	//sdpreset.setSettingsData(data);
	//suspicious semicolon - a good name for a band !

	sdpreset.getPatternData(settings->getCurrentPattern());
	stepper = createBastlStepper(settings->getPlayerMode());
	hardware.setResetState(settings->getPlayerMode() == PlayerSettings::MASTER);
	hardware.setMutes(settings->getInstrumentMuteByte());
	hardware.setTriggerLength(settings->getTriggerLength());

	player = new Player(&memory, settings, &synchronizer, &instrumentEvent);

	recorder.init(player, &memory, settings, stepper);

	SekvojModulePool::buttonMap_ = 		&buttonMap;
	SekvojModulePool::hw_ = 			&hardware;
	SekvojModulePool::player_ = 		player;
	SekvojModulePool::recorder_ = 		&recorder;
	SekvojModulePool::memory_ =			&memory;
	SekvojModulePool::settings_ = 		settings;
	SekvojModulePool::instrumentBar_ = 	&instrumentBar;
	SekvojModulePool::synchronizer_ = 	&synchronizer;
	SekvojModulePool::tapper_ = 		&tapper;
	SekvojModulePool::sd_ = 			&sdpreset;
	mainMenu.init(rstInCall);

	//Serial.begin(9600);
	//Serial.println("s");


	//Initialize tapping features
	//tapper.init(5000, 100); Not called rather made default to save some progeram memory
	//tapper.setStepsPerTap(16); Not called rather made default to save some progeram memory
	tapper.setStepCallBack(&tapStep);
	hardware.init(0, &clockInCall, &rstInCall);
	settingsChanged();
}

void loop() {
	//sdpreset.debug();
	//Update step keepers
	stepper->update(hardware.getElapsedBastlCycles());
	recorder.update();
	stepper->update(hardware.getElapsedBastlCycles());
	//Update user interface
	mainMenu.update();
	unsigned int bastlCycles = hardware.getElapsedBastlCycles();
	stepper->update(bastlCycles);
	player->update(bastlCycles);
	//stepper->update(bastlCycles);
}




