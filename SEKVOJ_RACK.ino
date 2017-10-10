
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
#include <LEDsAndButtonsHWWrapper.h>
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
InstrumentBar instrumentBar;
SekvojRackButtonMap buttonMap;
SimplifiedTapper tapper;
StepSwinger swinger;

unsigned char localStep = 0;

extern sekvojHW hardware;
unsigned int bastlCyclesPerSecond = 0;
unsigned int bastlCyclesLeftovers = 0;
bool slave = false;
unsigned char memoryData[290];

void stepperStep() {
	if (mainMenu.isPlaying()) {
		player->stepFourth();
		SekvojModulePool::synchronizer_.doStep();
		SekvojModulePool::recorder_.update();
		if (SekvojModulePool::synchronizer_.getCurrentStepNumber() % 4 == 0) {
			hardware.setTrigger(6, true,10);// ILEDsAndButtonsHW::TRIGGER_ON);
		}
	}
}

void instrumentEvent(unsigned char instrumentId, DrumStep::DrumVelocityType velocity, bool isOn) {

	//Serial.println(".");
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
	if (settings->getPlayerMode() == PlayerSettings::SLAVE) {
		stepper->doStep(hardware.getElapsedBastlCycles());
	}
}

void rstInCall() {
	player->resetAllInstruments();
	SekvojModulePool::synchronizer_.reset();
	stepper->reset();
}

void tapStep() {
	if (settings->getPlayerMode() == PlayerSettings::MASTER) {
		if (tapper.anyStepDetected()) {
			unsigned char timeUnitsPerStep = tapper.getTimeUnitsPerStep();
			((StepGenerator *)stepper)->setTimeUnitsPerStep(timeUnitsPerStep, 0);
			settings->setBPM(BPMConverter::timeUnitsToBPM(timeUnitsPerStep, bastlCyclesPerSecond), false);
		}
		stepper->doStep(hardware.getElapsedBastlCycles());
	}
}

void patternChanged(unsigned char originalPattenrIndex, unsigned char newPatternIndex) {
	SekvojModulePool::sd_.setPatternData(originalPattenrIndex);
	SekvojModulePool::sd_.getPatternData(newPatternIndex);

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
		unsigned int leftovers;
		unsigned int time = BPMConverter::bpmToTimeUnits(bpm, bastlCyclesPerSecond, bastlCyclesLeftovers, leftovers);
		((StepGenerator *)stepper)->setTimeUnitsPerStep(time, leftovers);
	}
}

BastlStepper * createBastlStepper(PlayerSettings::PlayerMode mode) {
	 if (mode == PlayerSettings::MASTER) {
	    StepGenerator * generator = new StepGenerator(&stepperStep, &swinger);
	    //generator->init(&stepperStep, BPMConverter::bpmToTimeUnits(settings->getBPM(), hardware.getBastlCyclesPerSecond()));
	    unsigned int leftovers;
	    unsigned int time = BPMConverter::bpmToTimeUnits(settings->getBPM(), bastlCyclesPerSecond, bastlCyclesLeftovers, leftovers);
	    generator->setTimeUnitsPerStep(time, leftovers);
	    return generator;
	  } else {
	    StepMultiplier * multiplier = new StepMultiplier(&stepperStep, &swinger, bastlCyclesPerSecond);
	    multiplier->setMultiplication(getMultiplicationFromEnum(settings->getMultiplication()));
	    return multiplier;
	  }
}

void playerModeChanged(PlayerSettings::PlayerMode mode) {
	player->resetAllInstruments();
	SekvojModulePool::synchronizer_.reset();
	if (stepper) {
		delete stepper;
	}
	stepper = createBastlStepper(mode);
	hardware.setResetState(mode == PlayerSettings::MASTER);
}

void settingsChanged() {
	unsigned char data[11];
	settings->getInByteArray(data);
	SekvojModulePool::sd_.setSettingsData(data);
	hardware.setMutes(settings->getInstrumentMuteByte());
	hardware.setTriggerLength(settings->getTriggerLength());
	instrumentBar.setInstrumentsMutes(settings->getInstrumentMuteByte());
	swinger.setSwing(settings->getSwing());

}

void setup() {

	hardware.init();

	swinger.init(& SekvojModulePool::synchronizer_);
	swinger.setSwing(0);

	bastlCyclesPerSecond = hardware.getBastlCyclesPerSecond(bastlCyclesLeftovers);
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
	SekvojModulePool::sd_.initCard(memoryData, data);
	SekvojModulePool::sd_.getSettingsData(data);
	//sdpreset.setSettingsData(data);
	settings->loadFromByteArray(data);
	//sdpreset.setSettingsData(data);
	//suspicious semicolon - a good name for a band !

	SekvojModulePool::sd_.getPatternData(settings->getCurrentPattern());
	stepper = createBastlStepper(settings->getPlayerMode());
	hardware.setResetState(settings->getPlayerMode() == PlayerSettings::MASTER);
	hardware.setMutes(settings->getInstrumentMuteByte());
	hardware.setTriggerLength(settings->getTriggerLength());

	player = new Player(&memory, settings, &SekvojModulePool::synchronizer_, &instrumentEvent);

	LEDsAndButtonsHWWrapper::hw_ = &hardware;

	SekvojModulePool::buttonMap_ = 		&buttonMap;
	SekvojModulePool::player_ = 			player;
	SekvojModulePool::memory_ =			&memory;
	SekvojModulePool::settings_ = 		settings;
	SekvojModulePool::instrumentBar_ = 	&instrumentBar;
	SekvojModulePool::tapper_ = 		&tapper;
	mainMenu.init(rstInCall);

	//Serial.begin(9600);
	//Serial.println("s");


	//Initialize tapping features
	//tapper.init(5000, 100); Not called rather made default to save some progeram memory
	//tapper.setStepsPerTap(16); Not called rather made default to save some progeram memory
	tapper.setStepCallBack(&tapStep);

	settingsChanged();

	hardware.setup(&clockInCall, &rstInCall);
}

void loop() {
	//sdpreset.debug();
	//Update step keepers
	stepper->update(hardware.getElapsedBastlCycles());
	SekvojModulePool::recorder_.update();
	stepper->update(hardware.getElapsedBastlCycles());
	//Update user interface
	mainMenu.update();
	unsigned int bastlCycles = hardware.getElapsedBastlCycles();
	stepper->update(bastlCycles);
	player->update(bastlCycles);
	//stepper->update(bastlCycles);
}




