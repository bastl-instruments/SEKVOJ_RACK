
#include "sekvojHW_settings.h"

#ifndef SEKVOJHW_H_
#define SEKVOJHW_H_

#include <IHWLayer.h>
#include <Arduino.h>


class sekvojHW : public IHWLayer {

enum DisplayDataType {COMMAND,DATA};

public:

	// sets up all the pins, timers and SPI interface
	// call this before using any other method from this class
	void init(void(*buttonChangeCallback)(uint8_t number));

	/***KNOBS***/

	//Disabled function in SEKVOJ
	virtual uint8_t getKnobValue(uint8_t index){return 0;};

	/***LEDS***/

	// set the state of a led
	virtual void setLED(uint8_t number, IHWLayer::LedState state);

	// set the state of a led
	virtual void setLED(uint8_t number, uint8_t number2, uint8_t number3 = 0) {}

	// print the state arrays to the Serial terminal
	void printLEDStates();


	/***BUTTONS***/

	// the the state of a button identified by its id
	virtual IHWLayer::ButtonState getButtonState(uint8_t number);

	// print the read button states to serial terminal
	void printButtonStates();


	/***RAM***/

	// write a byte to the given address
	virtual void writeSRAM(long address, uint8_t data);

	// write a number of bytes starting at the given address
	virtual void writeSRAM(long address, uint8_t* buf, uint16_t len);

	// read the byte stored at the given address
	virtual uint8_t readSRAM(long address);

	// read a number of bytes starting from the given address
	virtual void readSRAM(long address, uint8_t* buf, uint16_t len);



	/***DISPLAY***/

	// deletes all text on the display
	void clearDisplay();

	// sets the cursor to a given positon
	// after this, you can start writing text from that position
	void setDisplayCursor(uint8_t col, uint8_t row);

	// write a string to display
	// you can write an infitinte number of characters but only some of them are being dislayed at a time
	// this display can show two lines of 16 characters
	void writeDisplayText(char text[]);

	void writeDisplayNumber(uint8_t n);

	// convenient operator to write strings to display
	void operator <<(char text[]);

	// this operator lets you display special characters by their id
	// you can find the ids in the data sheet of HD44780 on page 17 (or 18 depending on the model you are using)
	void operator <<(uint8_t data);



	/**TIMING**/

	// the number of bastl cycles elapsed since startup
	// this number will overflow after some minutes; you have to deal with that in the layer above
	// using a longer datatype would prevent this but at the cost of longer computation time
	uint16_t getElapsedBastlCycles();

	// returns the relation between bastl cycles and seconds
	// this value is dependent on the hardware update frequency that you can set by a define
	// use this to map real time processes (like BMP) to bastlCycles
	uint16_t  getBastlCyclesPerSecond();




	// only called by ISR routine.
	// they would be declared private but this would prevent the ISR from accessing them
	// there are workarounds for this but as they come at a cost I just left it like this
	void isr_updateNextLEDRow();
	void isr_updateButtons();
	void isr_sendDisplayBuffer();
	inline void incrementBastlCycles() {bastlCycles++;}

	/**EEPROM**/

	virtual void readEEPROM(uint8_t pageNumb, uint8_t* byteArray, uint16_t numbBytes){}
	virtual void readEEPROMDirect(uint16_t address, uint8_t* byteArray, uint16_t numbBytes){}
	virtual bool isEEPROMBusy(){return true;}


private:
	/**TIMING**/
	uint16_t bastlCycles;

	/**LEDS**/
	uint16_t ledStatesBeg[4];
	uint16_t ledStatesEnd[4];

	/**BUTTONS**/
	uint16_t buttonStates[4];
	void (*buttonChangeCallback)(uint8_t number);

	/**DISPLAY**/

	// Sets up used pins and directly sends commands to set basic display mode
	// automatically called during sekvojHW::setup()
	void initDisplay();

	// Send a command or data byte directly (bypassing the display buffer) to the display
	// this needs to be used as long as the buffer is not read because the interrupt is not running
	void sendDisplayDirect(DisplayDataType dataType, uint8_t byte);

	// send a command or data byte to the display buffer
	// it will be read during the next interrupt
	void sendDisplay(DisplayDataType dataType, uint8_t byte);

	// writes a given byte to the display
	void sendByteToDisplay(uint8_t byte);

	volatile uint8_t displayBuffer;
	volatile bool    isDisplayBufferLoaded;

	uint8_t _displayfunction;
	uint8_t _displaycontrol;
	uint8_t _displaymode;




};

static inline __attribute__((always_inline)) byte spiRead() {
  SPDR = 0xFF; // start SPI clock
  while (!(SPSR & _BV(SPIF)));
  return SPDR;
}

static inline __attribute__((always_inline)) byte spiWrite(byte data) {

  SPDR = data;
  while (!(SPSR & _BV(SPIF)));

  return SPDR;
}





#endif
