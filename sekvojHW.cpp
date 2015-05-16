
/*** PERMANENT SETTINGS***/

#include "sekvojHW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "sekvojHW.h"
#include <shiftRegisterFast.h>
//#include <SdFat.h>

// Declaration of instance (for use in interrupt service routine)

sekvojHW hardware;

#define DEFAULT_TRIGGER_LENGTH 20
#define UINT16_MAX 65535
#define MAX_ADDR 131067

// set by defines
static const uint8_t updateFreq = FREQ;
static const uint8_t blinkCompare[2] = {blinkDuty,blinkTotal};

// set by hardware
// cols are numbers of elements that are read 'at the same time'
// rows are multiplexed in time
static const uint8_t leds_cols = 8;
static const uint8_t leds_rows = 4;
static const uint8_t buttons_cols = 4;
static const uint8_t buttons_rows = 8;

static const uint8_t rowsTotal = 4; // for calculation of update frequency timer

const uint8_t trigMap[6]={7,6,5,2,3,4};



void sekvojHW::init(void(*buttonChangeCallback)(uint8_t number),void(*clockInCallback)(),void(*rstInCallback)()) {

	cli();

	shiftRegFast::setup();

	// BUTTONS
	bit_dir_inp(BUTTONCOL_0);
	bit_dir_inp(BUTTONCOL_1);
	bit_dir_inp(BUTTONCOL_2);
	bit_dir_inp(BUTTONCOL_3);



	bit_dir_outp(LEDCOL_0);
	bit_dir_outp(LEDCOL_1);
	bit_dir_outp(LEDCOL_2);
	bit_dir_outp(LEDCOL_3);


	bit_set(BUTTONCOL_0);
	bit_set(BUTTONCOL_1);
	bit_set(BUTTONCOL_2);
	bit_set(BUTTONCOL_3);

	bit_clear(LEDCOL_0);
	bit_clear(LEDCOL_1);
	bit_clear(LEDCOL_2);
	bit_clear(LEDCOL_3);

	bit_dir_inp(CLOCK_IN_PIN);
	bit_clear(CLOCK_IN_PIN);

	bit_dir_outp(CLOCK_OUT_PIN);
	bit_clear(CLOCK_OUT_PIN);

	bit_dir_outp(RST_PIN);
	bit_clear(RST_PIN);


	// LEDS
	for (uint8_t row=0; row<leds_rows; row++) {
		ledStatesBeg[row] =  1<<(15-row);    				//set row hit high
		ledStatesBeg[row] |= (B00001111<<8) | (B11111111); //disable all rows

		ledStatesEnd [row] = ledStatesBeg[row]; 			// copy to second set of states
	}




	// store callback pointer for changed buttons
	 this->buttonChangeCallback = buttonChangeCallback;

	 this->clockInCallback = clockInCallback;
	 this->rstInCallback = rstInCallback;

	// Disable Timer1 interrupt
	//TIMSK1 &= ~_BV(TOIE1);

	// TIMER 2
	TCCR2A = (1 << WGM21);  // turn on CTC mode
	TIMSK2 |= (1 << OCIE2A);// enable interrupt
	TCCR2B = B00000111;	    //prescaler = 1024
	OCR2A = (F_CPU/1024)/(updateFreq*rowsTotal);
	TCNT2  = 0;


	sei();
}


/**** LEDS ****/

void sekvojHW::printLEDStates() {
	for (uint8_t row=0; row<leds_rows; row++) {
		Serial.print("Row "); Serial.print(row,DEC);Serial.print(": ");
		for (int8_t col=15; col>=0;col--) {
			if (bitRead(ledStatesBeg[row],col)) {
				Serial.print("1");
			} else {
				Serial.print("0");
			}
		}
		Serial.println("");
		Serial.print("    "); Serial.print(row,DEC);Serial.print(": ");
		for (int8_t col=15; col>=0;col--) {
			if (bitRead(ledStatesEnd[row],col)) {
				Serial.print("1");
			} else {
				Serial.print("0");
			}
		}
		Serial.println("");
	}
}



/*void sekvojHW::leds_setStates(uint16_t ledStates[]) {
	for (uint8_t row = 0; row<leds_rows; row++) {
		this->ledStatesBeg[row]=ledStates[row];

	}
}*/


void sekvojHW::setLED(uint8_t number, ILEDHW::LedState state) {

	uint8_t buttonGroupIndex = number / leds_cols;
	uint8_t oneAtInGroupIndex = 1 << (number % leds_cols);
	if ((state == IHWLayer::ON) | (state==IHWLayer::BLINK) | (state==IHWLayer::DULLON)) {
		ledStatesBeg[buttonGroupIndex] &= ~oneAtInGroupIndex;
	} else {
		ledStatesBeg[buttonGroupIndex] |= oneAtInGroupIndex;
	}

	if ((state == IHWLayer::ON) | (state== IHWLayer::BLINK_INVERT)) {
		ledStatesEnd[buttonGroupIndex] &= ~oneAtInGroupIndex;
	} else {
		ledStatesEnd[buttonGroupIndex] |= oneAtInGroupIndex;
	}
}



inline void sekvojHW::isr_updateNextLEDRow() {



	static uint8_t currentRow = 0;
	static uint8_t blinkCounter = 0;
	/*
	bit_clear(LEDCOL_0);
	bit_clear(LEDCOL_1);
	bit_clear(LEDCOL_2);
	bit_clear(LEDCOL_3);
	*/


	uint8_t * statesToWrite = (blinkCounter < blinkCompare[0]) ? ledStatesBeg : ledStatesEnd;
	shiftRegFast::write_8bit(statesToWrite[currentRow]);
	shiftRegFast::write_8bit(trigState);
	shiftRegFast::enableOutput();

	// go no next row


	switch(currentRow){

	case 0:
		bit_set(LEDCOL_0);
		break;
	case 1:
		bit_set(LEDCOL_1);
		break;
	case 2:
		bit_set(LEDCOL_2);
		break;
	case 3:
		bit_set(LEDCOL_3);
		break;

	}

	currentRow=(currentRow+1)%leds_rows;
	if (currentRow == 0) blinkCounter = (blinkCounter+1)%blinkCompare[1];
}



/**** BUTTONS ****/


inline void sekvojHW::isr_updateButtons() {

	bit_clear(LEDCOL_0);
	bit_clear(LEDCOL_1);
	bit_clear(LEDCOL_2);
	bit_clear(LEDCOL_3);


	for (int8_t row=7; row>=0; row--) {
		shiftRegFast::write_8bit(~(1<<row));
		shiftRegFast::write_8bit(trigState);
		shiftRegFast::enableOutput();


		uint8_t col = 0;

		bitWrite(buttonStates[col], row, !bit_read_in(BUTTONCOL_0));
		col++;
		bitWrite(buttonStates[col], row, !bit_read_in(BUTTONCOL_1));
		col++;
		bitWrite(buttonStates[col], row, !bit_read_in(BUTTONCOL_2));
		col++;
		bitWrite(buttonStates[col], row, !bit_read_in(BUTTONCOL_3));


		//col++;
	}

	/*if(buttonChangeCallback!=0){
		for(int col=0;col<4;col++){
			if(uint8_t xored=newButtonStates[col]^buttonStates[col]){
				for(int row=0;row<8;row++){
					if(bitRead(xored,row)) buttonChangeCallback(col*buttons_rows + row);
				}
			}
			buttonStates[col]=newButtonStates[col];
		}
	}
	for(int col=0;col<4;col++) buttonStates[col]=newButtonStates[col];*/
}


void sekvojHW::printButtonStates() {
	for (uint8_t row=0; row<4; row++) {
		Serial.print("col "); Serial.print(row,DEC);Serial.print(": ");
		for (int8_t col=7; col>=0;col--) {
			if (bitRead(buttonStates[row],col)) {
				Serial.print("1");
			} else {
				Serial.print("0");
			}
		}
		Serial.println("");
	}
}

bool sekvojHW::isButtonDown(uint8_t number) {
	return (buttonStates[number/buttons_rows] & (1<<(number%buttons_rows)));
}

IButtonHW::ButtonState sekvojHW::getButtonState(uint8_t number) {

	if (isButtonDown(number)) {
		return IButtonHW::DOWN;
	} else {
		return IButtonHW::UP;
	}

}


/**** TRIGGER ****/
void sekvojHW::setTrigger(uint8_t number, ILEDsAndButtonsHW::TriggerState state) {
	triggerCountdown[number] = (state == TRIGGER) ? DEFAULT_TRIGGER_LENGTH : 0;
	bitWrite(trigState, trigMap[number], (state == OFF) ? 1 : 0);
}

inline void sekvojHW::isr_updateTriggerStates(){
	for(int i = 0; i < 8; i++){
		if(triggerCountdown[i]>0){
			if(triggerCountdown[i]==1) setTrigger(i,OFF);
			triggerCountdown[i]--;
		}
	}
}

inline void sekvojHW::isr_updateClockIn(){
	if(clockInCallback!=0){
		static bool clockInState;
		bool newState=bit_read_in(CLOCK_IN_PIN);
		if(newState && !clockInState) clockInCallback();
		clockInState=newState;
	}
}

inline void sekvojHW::isr_updateClockOut(){
	if(bitRead(trigState,6)) bit_set(CLOCK_OUT_PIN);
	else bit_clear(CLOCK_OUT_PIN);
}

inline void sekvojHW::isr_updateReset(){
	if(rstMaster){
		bit_dir_outp(RST_PIN);
		if(bitRead(trigState,7)) bit_set(RST_PIN);
		else bit_clear(RST_PIN);
	}
	else{
		if(rstInCallback!=0){
			bit_dir_inp(RST_PIN);
			bit_clear(RST_PIN);
			static bool rstInState;
			bool newState=bit_read_in(RST_PIN);
			if(newState && !rstInState) rstInCallback();
			rstInState=newState;
		}
	}
}

/**** TIMING ****/

uint16_t sekvojHW::getElapsedBastlCycles() {
	return bastlCycles;
}

uint16_t sekvojHW::getBastlCyclesPerSecond() {
	return (F_CPU/1024)/OCR2A;
}


/**** INTERRUPT ****/

ISR(TIMER2_COMPA_vect) {


	//bit_set(PIN);
	hardware.incrementBastlCycles();
	//hardware.isr_sendDisplayBuffer();  // ~156us
	hardware.isr_updateTriggerStates();
	hardware.isr_updateButtons();      // ~1ms
	hardware.isr_updateNextLEDRow();   // ~84us
	hardware.isr_updateClockOut();
	hardware.isr_updateReset();
	hardware.isr_updateClockIn();

	//bit_clear(PIN);


}


