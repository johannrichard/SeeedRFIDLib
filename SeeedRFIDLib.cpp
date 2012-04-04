/**** 
 * Implementation of a number of methods
 * and functions for RFID Tags from SeeedStudio
 * (c) 2011, 2012 Johann Richard
 * Licensed under the MIT license
 * http://www.opensource.org/licenses/mit-license.php. 
 ****/
#include <SoftwareSerial.h>
#include "SeeedRFIDLib.h"
#include "Arduino.h"

volatile unsigned char SeeedRFIDLib::_databits[100];    // stores all of the data bits
volatile unsigned char SeeedRFIDLib::_bitCount;

// Initialise an UART Instance of the Library
SeeedRFIDLib::SeeedRFIDLib(int txPin, int rxPin) {
    _rfidIO = new SoftwareSerial(rxPin, txPin);
    _rfidIO->begin(9600);

    // Reset the message and other values
    _tag.mfr = 0;
    _tag.id = 0;
    _tag.chk = 0;
    _tag.valid = false;

    _idAvailable = false;
    _bytesRead = 0;
    _libType = RFID_UART;
}

// Initialize a Wiegand Interface
// This can only be used on PIN 2 (D0) & 3 (D1) 
// INT 0 and 1, respectively
// The dataLen Attribute specifies the number of bits to read 
// The Library currently only understands 26 bit or 35 bit length
SeeedRFIDLib::SeeedRFIDLib(int dataLen) {
    // Reset the message and other values
    _tag.mfr = 0;
    _tag.id = 0;
    _tag.chk = 0;
    _tag.valid = false;

    _idAvailable = false;
    _bytesRead = 0;
    _dataLen = dataLen;
    _libType = RFID_WIEGAND;
    
    resetWiegand();
    attachInterrupt(0, DATA0, FALLING);
    attachInterrupt(1, DATA1, FALLING);
}

// Returns the ID as a struct and sets the _idAvailable to "false"
RFIDTag SeeedRFIDLib::readId() {
	_idAvailable = false;
	resetWiegand();
	
#ifdef DEBUG
	Serial.println("readId()");
	Serial.println(_tag.raw);
	Serial.print("  MFR:\t");
	Serial.println(_tag.mfr,HEX);
	Serial.print("  ID:\t");
	Serial.println(_tag.id,HEX);
	Serial.print("  CHK:\t");
	Serial.println(_tag.chk,HEX);
#endif
	return _tag;
}

// Clears the interrupts. From https://github.com/aszymanik/Arduino-Wiegand-Interface/

void SeeedRFIDLib::resetWiegand() {
    // cleanup and get ready for the next card
    _bitCount = 0;
    _facilityCode = 0;
    _cardCode = 0;
    for (int i=0; i<MAX_BITS; i++) {
        _databits[i] = 0;
    }
}

// interrupt that happens when INTO goes low (0 bit)
void SeeedRFIDLib::DATA0() {
  //Serial.print("0");
  _bitCount++;
  
}

// interrupt that happens when INT1 goes low (1 bit)
void SeeedRFIDLib::DATA1() {
  //Serial.print("1");
  _databits[_bitCount] = 1;
  _bitCount++;
}


// Check the reader parity
boolean SeeedRFIDLib::checkParity26() {
      int i = 0;
      int evenCount = 0;
      int oddCount = 0;
      
      int evenBit = _databits[0];
      int oddBit = _databits[25];
      
      for(i = 1; i < 13; i++){
        if(_databits[i]&(0x01)){
          evenCount++;
        }
      }
      for(i = 13; i < 25; i++){
        if(_databits[i]&(0x01)){
          oddCount++;
        }
      }
      if(evenCount%2 == evenBit && oddCount%2 != oddBit){
        return true;
      }
      else{
        return false;
      }
}

// Read data, check whether a complete ID has been 
// read and return true if the ID can be read out
boolean SeeedRFIDLib::isIdAvailable() { 
    switch(_libType) {
        case RFID_UART:
            return isIdAvailableUART();
            break;
        case RFID_WIEGAND:
            return isIdAvailableWiegand();
            break;
        default: 
            return false;
            break;
    } 
}

boolean SeeedRFIDLib::isIdAvailableWiegand() {
    // No ID Available (yet), let's check!
    _idAvailable = false;
	_tag.mfr = 0;
	_tag.id  = 0;
	_tag.chk = 0;

    if (_bitCount == _dataLen && _dataLen == 35)
    {
      // 35 bit HID Corporate 1000 format
      // facility code = bits 2 to 14
      for (int i=2; i<14; i++)
      {
         _facilityCode <<=1;
         _facilityCode |= _databits[i];
      }
      
      // card code = bits 15 to 34
      for (int i=14; i<34; i++)
      {
         _cardCode <<=1;
         _cardCode |= _databits[i];
      }
      
      _tag.mfr = _facilityCode;
      _tag.id  = _cardCode;
      _tag.chk = 0;
      _tag.valid = _idAvailable = true;

#ifdef DEBUG
      Serial.print("FC = ");
      Serial.print(_facilityCode);
      Serial.print(", CC = ");
      Serial.println(_cardCode);
#endif
    }
    else if (_bitCount == _dataLen && _dataLen == 26)
    {
        // This is not really correct. 
        // With only 3 bytes, we have to limit 
        // ourselves to the Card Code
        // standard 26 bit format
        /*
        // facility code = bits 2 to 9
        for (int i=1; i<9; i++)
        {
             _facilityCode <<=1;
             _facilityCode |= _databits[i];
        }
        */
        
        // card code = bits 1 to 23
        for (int i=1; i<25; i++)
        {
             _cardCode <<=1;
             _cardCode |= _databits[i];
        }

        if(checkParity26()) {
            _tag.mfr = _facilityCode;
            _tag.id  = _cardCode;
            _tag.chk = 0;
            _tag.valid = _idAvailable = true;

#ifdef DEBUG
            Serial.print("FC = ");
            Serial.print(_facilityCode);
            Serial.print(", CC = ");
            Serial.println(_cardCode);
#endif
        }

    }
    
    return _idAvailable;
}

// Check for UART Data
boolean SeeedRFIDLib::isIdAvailableUART() {
	/**
	 * Method outline
	 *  1) read until a header byte is reached
	 *  2) whenever the method is called, put any 
	 *     new characters into the temporary store
	 *  3) whenever the final byte is reached, finalize 
	 *     the ID and return true, otherwise, return false
	 */
	char val;
	
	if(_rfidIO->available() > 0) {
      if((val = _rfidIO->read()) == 0x02) { 
	    // start reading input
	    _bytesRead = 0; 
	
		_tag.mfr = 0;
		_tag.id  = 0;
		_tag.chk = 0;
		_tag.valid = false;
		
		_idAvailable = false;
#ifdef DEBUG
		Serial.println("START");
#endif		
	  } else if(val == 0x03 && _bytesRead == 12) {
		// ID completely read
		byte checksum = 0;
		byte value = 0;
		String id = _tag.raw;
		
		_tag.mfr = hex2dec(id.substring(0,4));
		_tag.id  = hex2dec(id.substring(4,10));
		_tag.chk = hex2dec(id.substring(10,12));

		// Do checksum calculation
		int i2;		
		for(int i = 0; i < 5; i++) {
			i2 = 2*i;
			checksum ^= hex2dec(id.substring(i2,i2+2));
		}
#ifdef DEBUG
		Serial.println("VERIFICATION");
		Serial.print("  ID:\t");
		Serial.println(_tag.raw);
		Serial.print("  CHK:\t");
		Serial.println(checksum, HEX);
#endif		
		if (checksum == _tag.chk) {
			_tag.valid = _idAvailable = true;
#ifdef DEBUG
		Serial.println("VALID tag");
#endif		
		}
#ifdef DEBUG
		Serial.println("END");
#endif		
	  } else {
		_tag.raw[_bytesRead++] = val;
#ifdef DEBUG
		Serial.println("VALUE");
		Serial.println(_tag.raw);
#endif		
	  }
	}
	return _idAvailable;
}

// Convert a HEX String to a decimal value (up to 8 bytes (16 hex characters))
long SeeedRFIDLib::hex2dec(String hexCode) {
  char buf[19] = "";
  hexCode = "0x" + hexCode;
  hexCode.toCharArray(buf, 18);
#ifdef DEBUG
  Serial.print("Decoding ");
  Serial.print(hexCode);
  Serial.print(": ");
  Serial.println(strtol(buf, NULL, 0));
#endif
  return strtol(buf, NULL, 0);
}

	
