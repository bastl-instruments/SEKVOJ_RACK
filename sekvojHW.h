
#include "sekvojHW_settings.h"

#ifndef SEKVOJHW_H_
#define SEKVOJHW_H_

#include <IHWLayer.h>
#include <ILEDsAndButtonsHW.h>

#include <Arduino.h>


class sekvojHW : public ILEDsAndButtonsHW {



public:

	// sets up all the pins, timers and SPI interface
	// call this before using any other method from this class
	void init(void(*buttonChangeCallback)(uint8_t number), void(*clockInCallback)(),void(*rstInCallback)());

	/***KNOBS***/

	//Disabled function in SEKVOJ
	virtual uint8_t getKnobValue(uint8_t index){return 0;};

	/***LEDS***/

	// set the state of a led
	virtual void setLED(uint8_t number, ILEDHW::LedState state);

	// print the state arrays to the Serial terminal
	void printLEDStates();


	/***BUTTONS***/

	// the the state of a button identified by its id
	virtual IButtonHW::ButtonState getButtonState(uint8_t number);
	virtual bool isButtonDown(uint8_t number);

	// print the read button states to serial terminal
	void printButtonStates();
	/***TRIGGER***/
	//void setTrigger(uint8_t number, ILEDsAndButtonsHW::TriggerState state);
	virtual void setTrigger(uint8_t number, bool state, bool autoOff);


	/***RAM***/

	// write a byte to the given address
	virtual void writeSRAM(long address, uint8_t data){}

	// write a number of bytes starting at the given address
	virtual void writeSRAM(long address, uint8_t* buf, uint16_t len){}

	// read the byte stored at the given address
	virtual uint8_t readSRAM(long address){return 0;}

	// read a number of bytes starting from the given address
	virtual void readSRAM(long address, uint8_t* buf, uint16_t len){}




	/**TIMING**/

	// the number of bastl cycles elapsed since startup
	// this number will overflow after some minutes; you have to deal with that in the layer above
	// using a longer datatype would prevent this but at the cost of longer computation time
	uint32_t getElapsedBastlCycles();

	// returns the relation between bastl cycles and seconds
	// this value is dependent on the hardware update frequency that you can set by a define
	// use this to map real time processes (like BMP) to bastlCycles
	uint16_t  getBastlCyclesPerSecond();

	void setResetState(bool _rstMaster){ rstMaster=_rstMaster;};
	void setMutes(uint8_t  mutes);


	// only called by ISR routine.
	// they would be declared private but this would prevent the ISR from accessing them
	// there are workarounds for this but as they come at a cost I just left it like this
	void isr_updateNextLEDRow();
	void isr_updateButtons();
	void isr_updateTriggerStates();
	void isr_updateClockIn();

	void isr_updateClockOut();
	void isr_updateReset();

	inline void incrementBastlCycles() {bastlCycles++;}

	/**EEPROM**/

	virtual void readEEPROM(uint8_t pageNumb, uint8_t* byteArray, uint16_t numbBytes){}
	virtual void readEEPROMDirect(uint16_t address, uint8_t* byteArray, uint16_t numbBytes){}
	virtual bool isEEPROMBusy(){return true;}

private:

	/**TRIGGERS**/
	uint8_t trigMutesState;
	uint8_t trigState;
	uint8_t trigAutoOff;
	uint8_t triggerCountdown[8];
	uint8_t triggerBuffer[8];
	/**TIMING**/
	uint32_t bastlCycles;

	/**LEDS**/
	uint8_t ledStatesBeg[4];
	uint8_t ledStatesEnd[4];


	/**BUTTONS**/
	uint8_t newButtonStates[4];
	uint8_t buttonStates[4];
	void compareButtonStates();
	void (*buttonChangeCallback)(uint8_t number);
	void (*clockInCallback)();
	void (*rstInCallback)();


	bool rstMaster;





};




#endif
