/*** PERMANENT SETTINGS***/

#include "sekvojHW_settings.h"


/*** ACTUAL CODE ***/


#include <Arduino.h>
#include "sekvojHW.h"
#include <shiftRegisterFast.h>
#include <portManipulations.h>


// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// additional pins
#define RS_Pin C,2
#define ENABLE_Pin C,3

// hardware constants
const uint8_t numLines = 2;



void sekvojHW::initDisplay() {


	//// Interrupt is not running yet and therefore reading the display will not work
	//// --> use of sendDisplayCommandDirect to bypass buffer

	// set pins as output
	bit_dir_outp(RS_Pin);
	bit_dir_outp(ENABLE_Pin);
	bit_dir_outp(PIN);


	delayMicroseconds(50000); // wait for display to power on

	bit_clear(RS_Pin);        //set pins low to start commands
	bit_clear(ENABLE_Pin);


	// set display mode
	_displayfunction = LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS;

	sendDisplayDirect(COMMAND, LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms
    sendDisplayDirect(COMMAND, LCD_FUNCTIONSET |  _displayfunction);
    delayMicroseconds(150);
    sendDisplayDirect(COMMAND, LCD_FUNCTIONSET |  _displayfunction);
    sendDisplayDirect(COMMAND, LCD_FUNCTIONSET |  _displayfunction);

    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    sendDisplayDirect(COMMAND, LCD_DISPLAYCONTROL | _displaycontrol);

	sendDisplayDirect(COMMAND, LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	delayMicroseconds(2000);  			  // this command takes a long time!

    // initialize default direction
    sendDisplayDirect(COMMAND, LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);

    //_displaycontrol |= LCD_BLINKON;
    //sendDisplayDirect(COMMAND, LCD_DISPLAYCONTROL | _displaycontrol);

    sendDisplayDirect(COMMAND,LCD_CLEARDISPLAY);


}

void sekvojHW::sendDisplayDirect(DisplayDataType dataType, uint8_t byte) {
	if (dataType == COMMAND) bit_clear(RS_Pin);
	else                     bit_set(RS_Pin);

	sendByteToDisplay(byte);
}

void sekvojHW::sendDisplay(DisplayDataType dataType, uint8_t byte) {

	while (isDisplayBufferLoaded); //wait until buffer is free
	displayBuffer = byte;

	if (dataType == COMMAND) bit_clear(RS_Pin);
		else                     bit_set(RS_Pin);

	isDisplayBufferLoaded = true;


}


void sekvojHW::isr_sendDisplayBuffer() {
	if (isDisplayBufferLoaded) {
		sendByteToDisplay(displayBuffer);
		isDisplayBufferLoaded = false;
	}
}



void sekvojHW::sendByteToDisplay(uint8_t byte) {

	// switch off all the led rows and write the byte to display and latch it in
	shiftRegFast::write_8bit(byte,shiftRegFast::MSB_FIRST);
	shiftRegFast::write_8bit(B11110000,shiftRegFast::MSB_FIRST);
	shiftRegFast::enableOutput();

	bit_clear(ENABLE_Pin);
	delayMicroseconds(1);
	bit_set(ENABLE_Pin);
	delayMicroseconds(1);    // enable pulse must be >450ns
	bit_clear(ENABLE_Pin);
	delayMicroseconds(100);   // commands need > 37us to settle

}





void sekvojHW::setDisplayCursor(uint8_t col, uint8_t row){

  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row >= numLines ) {
    row = numLines-1;    // we count rows starting w/0
  }

  sendDisplay(COMMAND, LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void sekvojHW::writeDisplayText(char text[]) {

	while (*text != 0) {
		sendDisplay(DATA,(uint8_t)(*text));
		text++;
	}

}

void sekvojHW::operator <<(char text[]) {
	writeDisplayText(text);
}

void sekvojHW::operator <<(uint8_t data) {
	sendDisplay(DATA,data);
}


void sekvojHW::clearDisplay() {
	sendDisplay(COMMAND,LCD_CLEARDISPLAY);
}

void sekvojHW::writeDisplayNumber(uint8_t n) {

	const uint8_t maxDigits = 3;
	uint8_t lastValidIndex = 0;
	char tmp[maxDigits];


	do	{ 								/* generate digits in reverse order */
		tmp[lastValidIndex] = (n % 10 + '0'); 	/* get next digit */
		lastValidIndex++;
	} while ((n /= 10) > 0) ; 			/* delete it */

	char output [maxDigits+1];
	uint8_t outputIndex = 0;
	for (int8_t index = lastValidIndex-1; index>=0; index--) {
		output[outputIndex] = tmp[index];
		outputIndex++;
	}
	output[outputIndex] = 0;

	writeDisplayText(output);
}
