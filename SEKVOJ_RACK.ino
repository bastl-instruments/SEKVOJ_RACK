
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
#include <SdFat.h>
#include <SdFatUtil.h>

#include "SekvojRackMainMenuView.h"
#include "SekvojRackButtonMap.h"
#include <InstrumentBar.h>
#include <StepRecorder.h>
#include <StepSynchronizer.h>
#include <StepMultiplier.h>
#include <EEPROM.h>

//MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

Player * player;

ArduinoMIDICommandProcessor * processor;
NoVelocityStepMemory memory;
PlayerSettings * settings;

//------------------------------------------------------------------------------
// global variables
//Sd2Card card; // SD/SDHC card with support for version 2.00 features
//SdVolume vol; // FAT16 or FAT32 volume
//SdFile root; // volume's root directory
//SdFile file; // current file

Sd2Card card;           // SD/SDHC card with support for version 2.00 features
SdVolume vol;           // FAT16 or FAT32 volume
SdFile root;            // volume's root directory
SdFile file;

SekvojRackMainMenuView mainMenu;
StepGenerator stepper;
StepRecorder recorder;
InstrumentBar instrumentBar;
SekvojRackButtonMap buttonMap;
StepSynchronizer synchronizer;
StepMultiplier multiplier;

unsigned long lastBastlCycles = 0;
unsigned char localStep = 0;

extern sekvojHW hardware;
bool slave=false;

unsigned char memoryData[292];

void stepperStep() {
	/*localStep = (localStep + 1) % 64;
	for (unsigned char instrument = 0; instrument < 6; instrument++) {
		//if (instrument == 1) {
		DrumStep step = memory.getDrumStep(instrument, 0, localStep);
		//hardware.setLED(instrument, !step.isMuted() ? ILEDHW::ON : ILEDHW::OFF);
		instrumentBar.setInstrumentPlaying(instrument, !step.isMuted());
		//}
	}*/

	player->stepFourth();
	synchronizer.doStep();
}

void noteOn(unsigned char note, unsigned char velocity, unsigned char channel) {
	 //MIDI.sendNoteOn(35 + note, 127 ,channel);
	 //unsigned char instrumentIndex;
	 //if (settings->getDrumInstrumentIndexFromMIDIMessage(channel, note, instrumentIndex)) {
	 	instrumentBar.setInstrumentPlaying(channel, true);
	 	hardware.setTrigger(channel,sekvojHW::ON,20);
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
	//unsigned char instrumentIndex;
	//if (settings->getDrumInstrumentIndexFromMIDIMessage(channel, note, instrumentIndex)) {
		instrumentBar.setInstrumentPlaying(channel, false);
	//}
	//if (channel == 0)
		//instrumentBar.setInstrumentPlaying(channel, false);
}


void initFlashMemory(NoVelocityStepMemory * memory) {
	DrumStep::DrumVelocityType emptySteps[4] = {DrumStep::OFF, DrumStep::OFF, DrumStep::OFF, DrumStep::OFF};
	DrumStep::DrumVelocityType oneSteps[4] = {DrumStep::NORMAL, DrumStep::OFF, DrumStep::OFF, DrumStep::OFF};
	DrumStep emptyDrumStep(true, true, emptySteps);
	DrumStep oneDrumStep(true, false , oneSteps);

	for (unsigned char instrument = 0; instrument < 6; instrument++) {
		for (unsigned char step = 0; step < 64; step++) {
			memory->setDrumStep(instrument, step, emptyDrumStep);
			/*
			if (step % 6 == instrument) {
				memory->setDrumStep(instrument, 0, step, oneDrumStep);
			} else {

			}
			*/
		}
	}
}

void clockInCall(){
	multiplier.doStep(millis());
	slave=true;
}

void getPatternData(unsigned char patternIndex, unsigned char * data) {
	//timing needs optimisation ?
		// cca 60 ms opening file
			// cca 19 ms writing
		//cca 2 ms closing file
	Serial.println("load");
	uint32_t positionInFile=patternIndex*512;
	uint32_t time=millis();

	if (!file.open(&root, "PAT.txt", O_READ )) {

	}
	else{Serial.println(file.curPosition());
		file.seekSet(positionInFile);
		Serial.println(file.curPosition());
		Serial.println(file.read(&data[0],290));
		Serial.println(file.curPosition());
	}
	file.close();

    Serial.println(millis()-time);
    for (unsigned int dataIndex= 0; dataIndex < 288; dataIndex++) Serial.print(data[dataIndex]),Serial.print(" ,");
    Serial.println();
}

void setPatternData(unsigned char patternIndex, unsigned char * data) {
	Serial.println("store");
	uint32_t positionInFile=patternIndex*512;
	uint32_t time=millis();
	if (!file.open(&root, "PAT.txt", O_RDWR | O_CREAT )) {

	}
	else{
		//Serial.println(file.getFileSize());
		Serial.println(file.curPosition());
			file.seekSet(positionInFile);
			Serial.println(file.curPosition());
			Serial.println(file.write(&data[0],290));
			Serial.println(file.curPosition());
	}
	file.close();

    Serial.println(millis()-time);
    for (unsigned int dataIndex= 0; dataIndex < 288; dataIndex++) Serial.print(data[dataIndex]),Serial.print(" ,");
    Serial.println();
}
//<<<<<<< Updated upstream
void patternChanged(unsigned char patternIndex) {
	setPatternData(patternIndex,memoryData);
	getPatternData(patternIndex,memoryData);
	hardware.setLED(buttonMap.getMainMenuButtonIndex(4), ILEDHW::ON);
}
//=======


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
		settings->setDrumInstrumentEventType(i, PlayerSettings::TRIGGER);
	}

	//Here the pointer to the SD Card memory shall be set.
	memory.setDataReference(memoryData);
	initFlashMemory(&memory);

	processor = new ArduinoMIDICommandProcessor(&noteOn, &noteOff);
	player = new Player(&memory, processor, settings, &synchronizer);
	//Serial.end();
	//MIDI.begin(0);
	//MIDI.setHandleNoteOn(&midiNoteOnIn);

	//hardware.clearDisplay();

	recorder.init(player, &memory, settings);
	mainMenu.init(&hardware, player, & recorder, &memory, settings, processor, &instrumentBar, &buttonMap,  &synchronizer);
	//stepper.setTimeUnitsPerStep();
	Serial.begin(9600);
	multiplier.init(1000);//&stepperStep);
	multiplier.setMultiplication(16);
	multiplier.setMinTriggerTime(1);
	multiplier.setStepCallback(&stepperStep);
	Serial.println("strt");
	//if (!sd.begin(10, SPI_FULL_SPEED)) sd.initErrorHalt();

	if (!card.init()){Serial.println("int");};//error("card");
	if (!vol.init(&card)){Serial.println("crd");};// error("vol ");
	if (!root.openRoot(&vol)){Serial.println("vol");};// error("root");
	Serial.println("redy");
	//suspicious semicolon - a good name for a band !

	// file.createContiguous(&root, "PAT.txt", (64*512));
	// file.close();
/*
	if (!file.open(&root, "PAT.txt", O_RDWR | O_CREAT )) {
			//sd.errorHalt("opening test.txt for read failed");
			 Serial.println("er-write");
	}
	for(int i=0;i<(64*512);i++) file.write(255);
*/
	getPatternData(0,memoryData);


}





void loop() {
	/*
	for(int i=0;i<32;i++) {
		if(hardware.getButtonState(i)==IHWLayer::UP) hardware.setLED(i,IHWLayer::ON);
		else hardware.setLED(i,IHWLayer::OFF);
	}
	hardware.printButtonStates();
	*/
	//MIDI.read();
//	unsigned char someData[290];
	//if(hardware.getButtonState(0)==IHWLayer::DOWN) getPatternData(0,someData), setPatternData(0,someData);

	if(slave) multiplier.update(millis());
	else stepper.update(millis());//hardware.getElapsedBastlCycles());
	mainMenu.update();
	/*for (int i = 0; i < 16; i++) {
		if (hardware.getButtonState(i) == IButtonHW::DOWN) {
			hardware.setLED(i, ILEDHW::ON);
		} else {
			hardware.setLED(i, ILEDHW::OFF);
		}
	}*/
}




