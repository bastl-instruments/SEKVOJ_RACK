
/*** PERMANENT SETTINGS***/

#include "sekvojHW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "sekvojHW.h"
#include <shiftRegisterFast.h>
//#include <SdFat.h>

// Declaration of instance (for use in interrupt service routine)

sekvojHW hardware;

//In bastl cycles with perspective that one bastl cycle is 1.5ms
#define TRIGGER_LENGTH_1MS 1 	// 1 ms
#define TRIGGER_LENGTH_5MS 5 	// 5 ms
#define TRIGGER_LENGTH_10MS 10	// 10 ms
#define TRIGGER_LENGTH_20MS 20	// 20 ms
#define UINT16_MAX 65535
#define MAX_ADDR 131067

#define NOP asm volatile("nop\n\t")

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



//const uint8_t trigMap[8]={7,6,5,2,3,4,0,1};
uint8_t trigMap[8]={2,4,3,7,6,5,0,1};

uint8_t sekvojHW::getTriggerLength() {
	switch (trigLength) {
			case 0:
				return TRIGGER_LENGTH_1MS;
			case 1:
				return TRIGGER_LENGTH_5MS;
			case 2:
				return TRIGGER_LENGTH_10MS;
			case 3:
				return TRIGGER_LENGTH_20MS;
			case 4:
				return getRandom(TRIGGER_LENGTH_1MS, TRIGGER_LENGTH_5MS);
			case 5:
				return getRandom(TRIGGER_LENGTH_5MS, TRIGGER_LENGTH_10MS);
			case 6:
				return getRandom(TRIGGER_LENGTH_10MS, TRIGGER_LENGTH_20MS);
			case 7:
				return getRandom(TRIGGER_LENGTH_1MS, TRIGGER_LENGTH_20MS);
	}
}

void sekvojHW::setTriggerLength(uint8_t  triggerLength) {
	trigLength = triggerLength;
}

uint32_t sekvojHW::xorshift96() { //period 2^96-1
	// static unsigned long x=123456789, y=362436069, z=521288629;
	uint32_t t;

	rnd_x ^= rnd_x << 16;
	rnd_x ^= rnd_x >> 5;
	rnd_x ^= rnd_x << 1;

	t = rnd_x;
	rnd_x = rnd_y;
	rnd_y = rnd_z;
	rnd_z = t ^ rnd_x ^ rnd_y;

	return rnd_z;
}
uint8_t sekvojHW::getRandom(unsigned char min, unsigned char max) {
	return (unsigned char) ((((xorshift96() & 0xFFFF) * (max-min))>>16) + min);
}

void sekvojHW::setup(void(*clockInCallback)(),void(*rstInCallback)()) {

	// store callback pointer for changed buttons
	this->clockInCallback = clockInCallback;
	this->rstInCallback = rstInCallback;

}

void sekvojHW::init() {

	rnd_x=170;
	rnd_y=229;
	rnd_z=181;

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




	// LEDS
	/*for (uint8_t row=0; row<leds_rows; row++) {
		ledStatesBeg[row] =  1<<(15-row);    				//set row hit high
		ledStatesBeg[row] |= (B00001111<<8) | (B11111111); //disable all rows

		ledStatesEnd [row] = ledStatesBeg[row]; 			// copy to second set of states
	}*/


	 trigMutesState = 255;
	 trigLength = 0;

	// Disable Timer1 interrupt
	//TIMSK1 &= ~_BV(TOIE1);

	// TIMER 2
	TCCR2A = (1 << WGM21);  // turn on CTC mode
	TIMSK2 |= (1 << OCIE2A);// enable interrupt
	TCCR2B = B00000111;	    //prescaler = 1024
	OCR2A = 16;
	TCNT2  = 0;


	sei();
	bit_dir_outp(CLOCK_OUT_PIN);
	bit_clear(CLOCK_OUT_PIN);

	//bit_dir_outp(RST_PIN);
	//bit_clear(RST_PIN);
}

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

	uint8_t * statesToWrite = (blinkCounter < blinkCompare[0]) ? ledStatesBeg : ledStatesEnd;
	shiftRegFast::write_8bit(statesToWrite[currentRow]);
	shiftRegFast::write_8bit(trigState & trigMutesState);
	shiftRegFast::enableOutput();

	// go no next row

	switch(currentRow) {
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
	static uint8_t row=0;
	bit_clear(LEDCOL_0);
	bit_clear(LEDCOL_1);
	bit_clear(LEDCOL_2);
	bit_clear(LEDCOL_3);

    row = (row + 1) % 8;
	shiftRegFast::write_8bit(~(1<<row));
	shiftRegFast::write_8bit(trigState & trigMutesState);
	shiftRegFast::enableOutput();
	for (unsigned char i = 0; i < 10; i++) NOP;

	uint8_t col = 0;
	setBitWrapper(buttonStates[col], row, !bit_read_in(BUTTONCOL_0));
	col++;
	setBitWrapper(buttonStates[col], row, !bit_read_in(BUTTONCOL_1));
	col++;
	setBitWrapper(buttonStates[col], row, !bit_read_in(BUTTONCOL_2));
	col++;
	setBitWrapper(buttonStates[col], row, !bit_read_in(BUTTONCOL_3));
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

void sekvojHW::setBitWrapper(unsigned char &variable, unsigned char index, bool value) {
	bitWrite(variable, index, value);
}

/**** TRIGGER ****/
void sekvojHW::setTrigger(uint8_t number, bool state, bool autoOff){//ILEDsAndButtonsHW::TriggerState state) {

	setBitWrapper(trigAutoOff, number, autoOff);
	if (state)
		triggerBuffer[number]++;
	else {
		if (trigLength < 4) {
			setBitWrapper(trigState, trigMap[number], state);
		} else {
			triggerCountdown[number] = getTriggerLength() * 4;
			setBitWrapper(trigAutoOff, number, true);
		}
	}
}

void sekvojHW::setMutes(uint8_t  mutes){
	for (uint8_t i = 0; i < 6; i++) {
		setBitWrapper(trigMutesState, trigMap[i], bitRead(mutes, i));
	}
}

inline void sekvojHW::isr_updateTriggerStates(){
	for (uint8_t i = 0; i < 8; i++){
		if (triggerBuffer[i] != 0) {
			if (bitRead(trigState, trigMap[i]) && bitRead(trigAutoOff, i)) {
				setBitWrapper(trigState, trigMap[i], false);
			} else {
				setBitWrapper(trigState, trigMap[i], true);
				triggerCountdown[i] = (i == 6) ? 10 : getTriggerLength();
				triggerBuffer[i]--;
			}
		} else {
			if(triggerCountdown[i] > 0) {
				if (triggerCountdown[i] == 1 && bitRead(trigAutoOff, i)) {
					setBitWrapper(trigState, trigMap[i], 0);
				}
				triggerCountdown[i]--;
			}
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
	if(bitRead(trigState,0)) bit_set(CLOCK_OUT_PIN);
	else bit_clear(CLOCK_OUT_PIN);
}

inline void sekvojHW::isr_updateReset(){
	if(rstMaster){
		bit_dir_outp(RST_PIN);
		if(bitRead(trigState,1)) bit_set(RST_PIN);
		else bit_clear(RST_PIN);
	}
	else{
		if(rstInCallback!=0){
			bit_dir_inp(RST_PIN);
			bit_set(RST_PIN);
			static bool rstInState;
			bool newState=bit_read_in(RST_PIN);
			if(newState && !rstInState) rstInCallback();
			rstInState=newState;
		}
	}
}

/**** TIMING ****/

uint16_t sekvojHW::getElapsedBastlCycles() {
	noInterrupts();
	long cycles = bastlCycles;
	interrupts();
	return cycles;
}

uint16_t sekvojHW::getBastlCyclesPerSecond(unsigned int & leftovers) {
	unsigned int divider = (1024 * 17);
	unsigned int value = F_CPU / divider;
	leftovers = ((F_CPU % divider) * 1000) / divider;
	return value;
}


/**** INTERRUPT ****/

ISR(TIMER2_COMPA_vect) { // 80uS (used to update all 8 rows which took 640uS)
	hardware.incrementBastlCycles();
	hardware.isr_updateTriggerStates(); //8uS
	hardware.isr_updateButtons();      // 74uS (used to update all 8 rows which took 560uS)
	hardware.isr_updateNextLEDRow();   // ~84us
	hardware.isr_updateClockOut();
	hardware.isr_updateReset();
	hardware.isr_updateClockIn();
}


