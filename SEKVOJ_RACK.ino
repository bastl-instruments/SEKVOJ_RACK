
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
//#include "SekvojRackSDPreset.h"
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
//SekvojRackSDPreset sdpreset;
SimplifiedTapper tapper;

unsigned char localStep = 0;

extern sekvojHW hardware;
bool slave = false;
bool tapButtonDown = false;
unsigned char memoryData[292];

void stepperStep() {
	if (mainMenu.isPlaying()) {
		player->stepFourth();
		synchronizer.doStep();
		recorder.update();
	}
}

void noteOn(unsigned char note, unsigned char velocity, unsigned char channel) {
	instrumentBar.setInstrumentPlaying(channel, true);
	if (settings->getDrumInstrumentEventType(channel)==PlayerSettings::GATE){
		hardware.setTrigger(channel,sekvojHW::ON);
	} else if (settings->getDrumInstrumentEventType(channel)==PlayerSettings::TRIGGER) {
		hardware.setTrigger(channel,sekvojHW::ON,20);
	}
}

void noteOff(unsigned char note, unsigned char velocity, unsigned char channel) {
	instrumentBar.setInstrumentPlaying(channel, false);
	if (settings->getDrumInstrumentEventType(channel) == PlayerSettings::GATE){
		hardware.setTrigger(channel, sekvojHW::OFF);
	}
}


void initFlashMemory(NoVelocityStepMemory * memory) {
	memory->makeAllInstrumentsActiveUpTo(15);
	memory->clearStepsForAllInstruments();
}

void clockInCall() {
	if (settings->getPlayerMode() == PlayerSettings::SLAVE && mainMenu.isPlaying()) {
		stepper->doStep(hardware.getElapsedBastlCycles());
	}
}

void tapStep() {
	if (settings->getPlayerMode() == PlayerSettings::MASTER) {
		if (tapper.anyStepDetected()) {
			((StepGenerator *)stepper)->setTimeUnitsPerStep(tapper.getTimeUnitsPerStep());
			settings->setBPM(BPMConverter::timeUnitsToBPM(tapper.getTimeUnitsPerStep(), hardware.getBastlCyclesPerSecond()), false);
		}
		stepper->doStep(hardware.getElapsedBastlCycles());
	}
}

void patternChanged(unsigned char patternIndex) {
	//sdpreset.setPatternData(sdpreset.getCurrentPattern(),memoryData);
	//sdpreset.getPatternData(patternIndex,memoryData);

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
		((StepGenerator *)stepper)->setTimeUnitsPerStep(BPMConverter::bpmToTimeUnits(bpm, hardware.getBastlCyclesPerSecond()));
	}
}

BastlStepper * createBastlStepper(PlayerSettings::PlayerMode mode) {

	if (mode == PlayerSettings::MASTER) {
		StepGenerator * generator = new StepGenerator();
		generator->setStepCallback(&stepperStep);
		generator->setTimeUnitsPerStep(BPMConverter::bpmToTimeUnits(settings->getBPM(), hardware.getBastlCyclesPerSecond()));
		return generator;
	} else {
		StepMultiplier * multiplier = new StepMultiplier();
		multiplier->init(hardware.getBastlCyclesPerSecond());
		multiplier->setMultiplication(getMultiplicationFromEnum(settings->getMultiplication()));
		multiplier->setMinTriggerTime(1);
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

	hardware.init(0, &clockInCall);

	synchronizer.setCycleLength(256);
	stepper = createBastlStepper(settings->getPlayerMode());

	instrumentBar.init(&hardware, &buttonMap, 6);

	settings = new PlayerSettings();
	settings->setCurrentPattern(0);
	settings->setPatternChangedCallback(&patternChanged);
	settings->setMultiplicationChangedCallback(&multiplicationChanged);
	settings->setBPMChangedCallback(&bpmChanged);
	settings->setBPM(120);
	settings->setPlayerModeChangedCallback(&playerModeChanged);

	for (unsigned char i = 0; i < 6; i++) {
		settings->setInstrumentOn(Step::DRUM, i, true);
		settings->setInstrumentChannel(Step::DRUM, i, i);
		settings->setDrumInstrumentNote(i, i);
		settings->setDrumInstrumentEventType(i, PlayerSettings::TRIGGER); // TODO: load from some memory
	}

	//Here the pointer to the SD Card memory shall be set.
	memory.setDataReference(memoryData);
	initFlashMemory(&memory);

	processor = new ArduinoMIDICommandProcessor(&noteOn, &noteOff);
	player = new Player(&memory, processor, settings, &synchronizer);

	recorder.init(player, &memory, settings, stepper);
	mainMenu.init(&hardware, player, & recorder, &memory, settings, &instrumentBar, &buttonMap,  &synchronizer);

	//Serial.begin(9600);
	//Serial.println("s");

	//sdpreset.initCard(memoryData);
	//suspicious semicolon - a good name for a band !

	//	sdpreset.getPatternData(0,memoryData);

	//Initialize tapping features
	tapper.init(5000, 100);
	tapper.setStepsPerTap(16);
	tapper.setStepCallBack(&tapStep);
}

void loop() {

	//Tap the tapper in case tap button has been just pressed
	bool newTapButonDown = hardware.getButtonState(buttonMap.getMainMenuButtonIndex(0)) == IButtonHW::DOWN;
	if (!tapButtonDown && newTapButonDown) {
		tapper.tap(hardware.getElapsedBastlCycles());
	}

	tapButtonDown = newTapButonDown;

	//Update step keepers
	stepper->update(hardware.getElapsedBastlCycles());

	//Update user interface
	mainMenu.update();

	hardware.setLED(buttonMap.getMainMenuButtonIndex(0), synchronizer.getCurrentStepNumber() % 16 == 0 && mainMenu.isPlaying() ? ILEDHW::ON : ILEDHW::OFF);
}




