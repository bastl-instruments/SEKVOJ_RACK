
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

unsigned char localStep = 0;

extern sekvojHW hardware;
unsigned int bastlCyclesPerSecond = 0;
bool slave = false;
unsigned char memoryData[292];

void stepperStep() {
	if (mainMenu.isPlaying()) {
		player->stepFourth();
		synchronizer.doStep();
		recorder.update();
		if (synchronizer.getCurrentStepNumber() % 4 == 0) {
			hardware.setTrigger(6, true,20);// ILEDsAndButtonsHW::TRIGGER_ON);
		}
	}
}

void instrumentEvent(unsigned char instrumentId, DrumStep::DrumVelocityType velocity, bool isOn) {
	instrumentBar.setInstrumentPlaying(instrumentId, isOn);
	if (isOn) {
		if (settings->getDrumInstrumentEventType(instrumentId) == PlayerSettings::GATE) {
			hardware.setTrigger(instrumentId, true,0);//ILEDsAndButtonsHW::GATE_ON);
		} else {
			hardware.setTrigger(instrumentId, true,20);//ILEDsAndButtonsHW::TRIGGER_ON);
		}
	} else if (settings->getDrumInstrumentEventType(instrumentId) == PlayerSettings::GATE){
		hardware.setTrigger(instrumentId, false,0);//ILEDsAndButtonsHW::GATE_OFF);
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
		StepGenerator * generator = new StepGenerator();
		//generator->init(&stepperStep, BPMConverter::bpmToTimeUnits(settings->getBPM(), hardware.getBastlCyclesPerSecond()));
		generator->setStepCallback(&stepperStep);
		generator->setTimeUnitsPerStep(BPMConverter::bpmToTimeUnits(settings->getBPM(), bastlCyclesPerSecond));
		return generator;
	} else {
		StepMultiplier * multiplier = new StepMultiplier();
		multiplier->init(getMultiplicationFromEnum(settings->getMultiplication()), 1, bastlCyclesPerSecond);
		multiplier->setStepCallback(&stepperStep);
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
	unsigned char data[8];
	settings->getInByteArray(data);
	sdpreset.setSettingsData(data);
}

void setup() {

	hardware.init(0, &clockInCall, &rstInCall);
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

	unsigned char data[8];
	settings->getInByteArray(data);
	sdpreset.initCard(memoryData, data);
	sdpreset.getSettingsData(data);
	settings->loadFromByteArray(data);

	//suspicious semicolon - a good name for a band !

	sdpreset.getPatternData(settings->getCurrentPattern());
	stepper = createBastlStepper(settings->getPlayerMode());
	hardware.setResetState(settings->getPlayerMode() == PlayerSettings::MASTER);

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
	mainMenu.init();

	//Serial.begin(9600);
	//Serial.println("s");


	//Initialize tapping features
	//tapper.init(5000, 100); Not called rather made default to save some progeram memory
	//tapper.setStepsPerTap(16); Not called rather made default to save some progeram memory
	tapper.setStepCallBack(&tapStep);
}

void loop() {
	//sdpreset.debug();

	unsigned int bastlCycles = hardware.getElapsedBastlCycles();

	//Update step keepers
	stepper->update(bastlCycles);
	recorder.update();

	//Update user interface
	mainMenu.update();
	player->update(bastlCycles);
}




