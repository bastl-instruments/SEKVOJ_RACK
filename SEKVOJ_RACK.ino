
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
#include "SekvojRackSDPreset.h"
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
unsigned int bastlCyclesPerSecond = hardware.getBastlCyclesPerSecond();
bool slave = false;
unsigned char memoryData[292];
//unsigned char copyMemoryData[292];

void stepperStep() {
	if (mainMenu.isPlaying()) {
		player->stepFourth();
		synchronizer.doStep();
		recorder.update();
	}
}

void instrumentEvent(unsigned char instrumentId, DrumStep::DrumVelocityType velocity, bool isOn) {
	instrumentBar.setInstrumentPlaying(instrumentId, isOn);
	if (isOn) {
		if (settings->getDrumInstrumentEventType(instrumentId) == PlayerSettings::GATE) {
			hardware.setTrigger(instrumentId, sekvojHW::ON);
		} else {
			hardware.setTrigger(instrumentId, sekvojHW::ON, 20);
		}
	} else if (settings->getDrumInstrumentEventType(instrumentId) == PlayerSettings::GATE){
		hardware.setTrigger(instrumentId, sekvojHW::OFF);
	}
}

void clockInCall() {
	if (settings->getPlayerMode() == PlayerSettings::SLAVE && mainMenu.isPlaying()) {
		stepper->doStep(hardware.getElapsedBastlCycles());
	}
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

void patternChanged(unsigned char patternIndex) {
	sdpreset.setPatternData(sdpreset.getCurrentPattern(),memoryData);
	sdpreset.getPatternData(patternIndex,memoryData);

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
		multiplier->init(getMultiplicationFromEnum(settings->getMultiplication()), bastlCyclesPerSecond, 1);
		multiplier->setStepCallback(&stepperStep);
		return multiplier;
	}
}

void playerModeChanged(PlayerSettings::PlayerMode mode) {
	player->resetAllInstruments();
	if (stepper) {
		delete stepper;
	}
	stepper = createBastlStepper(mode);
}

void setup() {

	/*for (unsigned int i = 0; i < 292; i++) {
		if (i == 0) {
			copyMemoryData[i] = 0;
		} else {
			copyMemoryData[i] = (copyMemoryData[i - 1] + 1) % 10;
		}
	}*/
	hardware.init(0, &clockInCall);

	//synchronizer.setCycleLength(256); rather made as default to save some memory
	stepper = createBastlStepper(settings->getPlayerMode());

	instrumentBar.init(&hardware, &buttonMap, 6);

	settings = new PlayerSettings();
	settings->setPatternChangedCallback(&patternChanged);
	settings->setMultiplicationChangedCallback(&multiplicationChanged);
	settings->setBPMChangedCallback(&bpmChanged);
	settings->setBPM(120);
	settings->setPlayerModeChangedCallback(&playerModeChanged);

	for (unsigned char i = 0; i < 6; i++) {
		settings->setInstrumentOn(Step::DRUM, i, true);
		settings->setDrumInstrumentEventType(i, PlayerSettings::TRIGGER); // TODO: load from some memory
	}

	//Here the pointer to the SD Card memory shall be set.
	memory.setDataReference(memoryData);
	memory.makeAllInstrumentsActiveUpTo(15);
	memory.clearStepsForAllInstruments();
	player = new Player(&memory, settings, &synchronizer, &instrumentEvent);

	recorder.init(player, &memory, settings, stepper);
	mainMenu.init(&hardware, player, & recorder, &memory, settings, &instrumentBar, &buttonMap,  &synchronizer, &tapper);

	//Serial.begin(9600);
	//Serial.println("s");

	sdpreset.initCard(memoryData);
	//suspicious semicolon - a good name for a band !

	sdpreset.getPatternData(0,memoryData);

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

	//Update user interface
	mainMenu.update();

	hardware.setLED(buttonMap.getJumpButtonIndex(), synchronizer.getCurrentStepNumber() % 16 == 0 && mainMenu.isPlaying() ? ILEDHW::ON : ILEDHW::OFF);

}




