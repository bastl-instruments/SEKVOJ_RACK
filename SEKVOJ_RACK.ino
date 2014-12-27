
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
StepGenerator stepper;
StepRecorder recorder;
InstrumentBar instrumentBar;
SekvojRackButtonMap buttonMap;
StepSynchronizer synchronizer;
StepMultiplier multiplier;
SekvojRackSDPreset sdpreset;
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
	DrumStep::DrumVelocityType emptySteps[4] = {DrumStep::OFF, DrumStep::OFF, DrumStep::OFF, DrumStep::OFF};
	DrumStep emptyDrumStep(true, true, emptySteps);
	DrumStep emptyNonActiveDrumStep(false, true, emptySteps);

	for (unsigned char instrument = 0; instrument < 6; instrument++) {
		for (unsigned char step = 0; step < 64; step++) {
			if (step < 16) {
				memory->setDrumStep(instrument, step, emptyDrumStep);
			} else {
				memory->setDrumStep(instrument, step, emptyNonActiveDrumStep);
			}
		}
	}
}

void clockInCall(){
	multiplier.doStep(millis());
	slave = true;
	recorder.setCurrentStepper(&multiplier);
}

void tapStep() {
	if (tapper.anyStepDetected()) {
		stepper.setTimeUnitsPerStep(tapper.getTimeUnitsPerStep());
	}
	stepper.doStep(hardware.getElapsedBastlCycles());
}

void patternChanged(unsigned char patternIndex) {
	sdpreset.setPatternData(sdpreset.getCurrentPattern(),memoryData);
	sdpreset.getPatternData(patternIndex,memoryData);

}

void setup() {

	hardware.init(0, &clockInCall);

	synchronizer.setCycleLength(256);

	instrumentBar.init(&hardware, &buttonMap, 6);
	stepper.setTimeUnitsPerStep(BPMConverter::bpmToTimeUnits(120,1000));// hardware.getBastlCyclesPerSecond()));
	stepper.setStepCallback(&stepperStep);

	settings = new PlayerSettings();
	settings->setCurrentPattern(0);
	settings->setPatternChangedCallback(&patternChanged);

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

	recorder.init(player, &memory, settings, &stepper);
	mainMenu.init(&hardware, player, & recorder, &memory, settings, processor, &instrumentBar, &buttonMap,  &synchronizer);

	multiplier.init(1000);
	multiplier.setMultiplication(16);
	multiplier.setMinTriggerTime(1);
	multiplier.setStepCallback(&stepperStep);

	//Serial.begin(9600);
	//Serial.println("s");

	sdpreset.initCard(memoryData);
	//suspicious semicolon - a good name for a band !

	sdpreset.getPatternData(0,memoryData);

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
<<<<<<< HEAD
	hardware.printButtonStates();
	*/
	//MIDI.read();
//	unsigned char someData[290];
	//if(hardware.getButtonState(0)==IHWLayer::DOWN) getPatternData(0,someData), setPatternData(0,someData);

	//playButtonAction();
	if(slave) multiplier.update(millis());
	else stepper.update(millis());
	//stepper.update(millis());//hardware.getElapsedBastlCycles());
=======
	tapButtonDown = newTapButonDown;

	//Update step keepers
	if (slave) {
		multiplier.update(hardware.getElapsedBastlCycles());
	} else {
		stepper.update(hardware.getElapsedBastlCycles());
	}

	//Update user interface
>>>>>>> FETCH_HEAD
	mainMenu.update();
}




