
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
//#include <Player.h>
//#include <StepGenerator.h>
//#include <BPMConverter.h>
//#include <ArduinoMIDICommandProcessor.h>
//#include <PlayerSettings.h>
#include <NoVelocityStepMemory.h>
#include <RackInstrumentDefinitions.h>
//#include <MIDI.h>
//#include <SdFat.h>
//#include "MainMenuView.h"
#include "SekvojRackButtonMap.h"
#include <InstrumentBar.h>
//#include <StepRecorder.h>
//#include <StepSynchronizer.h>

//MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

//Player * player;

//ArduinoMIDICommandProcessor * processor;
NoVelocityStepMemory memory;
//PlayerSettings * settings;

//------------------------------------------------------------------------------
// global variables
//Sd2Card card; // SD/SDHC card with support for version 2.00 features
//SdVolume vol; // FAT16 or FAT32 volume
//SdFile root; // volume's root directory
//SdFile file; // current file

//MainMenuView mainMenu;
//StepGenerator stepper;
//StepRecorder recorder;
//InstrumentBar instrumentBar;
SekvojRackButtonMap buttonMap;
//StepSynchronizer synchronizer;

unsigned long lastBastlCycles = 0;
unsigned int localStep = 0;

extern sekvojHW hardware;
bool lastOn = false;


void stepperStep() {
	//player->stepFourth();
	//synchronizer.doStep();
}

void noteOn(unsigned char note, unsigned char velocity, unsigned char channel) {
	 //MIDI.sendNoteOn(35 + note, 127 ,channel);
	 //unsigned char instrumentIndex;
	 //if (settings->getDrumInstrumentIndexFromMIDIMessage(channel, note, instrumentIndex)) {
	 //	instrumentBar.setInstrumentPlaying(instrumentIndex, true);
	 //}
	//if (channel == 0)
	 //instrumentBar.setInstrumentPlaying(channel, true);
	 //hardware.clearDisplay();
	 //hardware.writeDisplayNumber(note * 10);

}

void noteOff(unsigned char note, unsigned char velocity, unsigned char channel) {

	//MIDI.sendNoteOff(35 + note, velocity ,channel);
	//hardware.clearDisplay();
	//hardware.writeDisplayNumber(note * 10 + 1);
	/*unsigned char instrumentIndex;
	if (settings->getDrumInstrumentIndexFromMIDIMessage(channel, note, instrumentIndex)) {
		instrumentBar.setInstrumentPlaying(instrumentIndex, false);
	}*/
	//if (channel == 0)
		//instrumentBar.setInstrumentPlaying(channel, false);
}


void initFlashMemory(NoVelocityStepMemory * memory) {
	DrumStep::DrumVelocityType inactiveSteps[4] = {DrumStep::OFF, DrumStep::OFF, DrumStep::OFF, DrumStep::OFF};
	DrumStep::DrumVelocityType activeSteps[4] = {DrumStep::NORMAL, DrumStep::OFF, DrumStep::NORMAL, DrumStep::OFF};
	DrumStep::DrumVelocityType activeSteps2[4] = {DrumStep::OFF, DrumStep::NORMAL, DrumStep::OFF, DrumStep::OFF};
	DrumStep::DrumVelocityType activeSteps3[4] = {DrumStep::OFF, DrumStep::OFF, DrumStep::NORMAL, DrumStep::OFF};
	DrumStep activeDrumStep(true, true, activeSteps);
	DrumStep emptyDrumStep(true, false , activeSteps);
	DrumStep emptyDrumStep2(true, false , activeSteps2);
	DrumStep emptyDrumStep3(true, false , activeSteps3);
	DrumStep muteDrumStep(true, true, inactiveSteps);

	// Initialize memory to empty
	DrumStep inactiveDrumStep(false, false, inactiveSteps);
	for (unsigned char instrument = 0; instrument < 6; instrument++) {
		for (unsigned char step = 0; step < 64; step++) {
			memory->setDrumStep(instrument, 0, step, activeDrumStep);
		}
	}
}

void setup() {

	hardware.init(0, 0);

	lastBastlCycles = hardware.getElapsedBastlCycles();



	//instrumentBar.init(&hardware, &buttonMap, 1);
	//stepper.setTimeUnitsPerStep(BPMConverter::bpmToTimeUnits(140, hardware.getBastlCyclesPerSecond()));
	//stepper.setStepCallback(&stepperStep);

	//settings = new PlayerSettings();
	//settings->setCurrentPattern(0);

	/*for (unsigned char i = 0; i < 1; i++) {
		settings->setInstrumentOn(Step::DRUM, i, true);
		settings->setInstrumentChannel(Step::DRUM, i, i);
		settings->setDrumInstrumentNote(i, i);
	}*/


	initFlashMemory(&memory);

	//processor = new ArduinoMIDICommandProcessor(&noteOn, &noteOff);
	//player = new Player(&memory, processor, settings, &synchronizer);
	//Serial.end();
	//MIDI.begin(0);
	//MIDI.setHandleNoteOn(&midiNoteOnIn);

	//hardware.clearDisplay();

	//recorder.init(player, memory, settings);
	//mainMenu.init(&hardware, player, & recorder, memory, settings, processor, &instrumentBar, &buttonMap);

	//Serial.begin(9600);
}



void loop() {

	if (hardware.getElapsedBastlCycles() - lastBastlCycles > 100) {
		lastOn = !lastOn;
		hardware.setLED(10, lastOn ? IHWLayer::ON : IHWLayer::OFF);
		localStep = (localStep + 1) % 256;
		lastBastlCycles = hardware.getElapsedBastlCycles();
		unsigned char readInstruments = 0;
		for (unsigned char instrument = 0; instrument < 6; instrument++) {
			DrumStep step = memory.getDrumStep(instrument, 0, 63 - localStep);
			readInstruments = readInstruments + step.isActive() ? 1 : 0;
			bool isOn = step.getSubStep(localStep % 4) == DrumStep::NORMAL;
			hardware.setLED(instrument, isOn ? IHWLayer::ON : IHWLayer::OFF);
		}
		hardware.setLED(readInstruments, lastOn ? IHWLayer::ON : IHWLayer::OFF);
	}
	/*
	for(int i=0;i<32;i++) {
		if(hardware.getButtonState(i)==IHWLayer::UP) hardware.setLED(i,IHWLayer::ON);
		else hardware.setLED(i,IHWLayer::OFF);
	}
	hardware.printButtonStates();
	*/
	//MIDI.read();
	//stepper.update(hardware.getElapsedBastlCycles());
	//mainMenu.update();
}




