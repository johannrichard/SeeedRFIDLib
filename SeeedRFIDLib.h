/**
 * Definitions and methods for RFID tag scanning and 
 * the whole shebang of getting the tags out of the reader
 * (c) 2011, 2012 Johann Richard
 * Licensed under the MIT license
 * http://www.opensource.org/licenses/mit-license.php.
 */
#ifndef SeeedRFIDLib_h
#define SeeedRFIDLib_h

// #define DEBUG true // Use this if you want to debug your RFID stuff

#include <SoftwareSerial.h>
#include "Arduino.h"

/**** 
 * Struct for storing an RFID tag
 */
struct RFIDTag {
	int mfr;         // Manufacturer (?) Code (2 bytes), only useful in UART Mode
	long id;         // Tag ID (3 bytes)
	byte chk;        // Checksum (1 byte), only useful in UART Mode
	boolean valid;   // Validity of the Tag, based on the Checksum (UART Mode) or the parity bits (Wiegand Mode)
	char raw[13];    // The whole tag as a raw string, only useful in UART Mode
};

enum RFIDLibType {
    RFID_UART,
    RFID_WIEGAND
};

#define MAX_BITS 100

// RFID Electronic Brick from Seeedstudio has 26 Bit Wiegand
#define SEEED_EBRICK_RFID 26
#define WIEGAND_26BIT 26
#define WIEGAND_35BIT 35
/***
 * Class for reading and checking RFID tags (UART Mode)
 */
class SeeedRFIDLib
{
private:
	SoftwareSerial * _rfidIO;
	RFIDTag _tag;
    RFIDLibType _libType;
	byte _bytesRead;
    byte _dataLen;
	boolean _idAvailable;
    volatile unsigned static char _databits[MAX_BITS];    // stores all of the data bits
    volatile unsigned static char _bitCount;
    unsigned long _facilityCode;        // decoded facility code
    unsigned long _cardCode;
    void resetWiegand();
    static void DATA0(void);
    static void DATA1(void);
    boolean checkParity26();
	boolean isIdAvailableUART();
	boolean isIdAvailableWiegand();
public: 
    SeeedRFIDLib(int rxPin, int txPin);
	SeeedRFIDLib(int dataLen);
	boolean isIdAvailable();
	RFIDTag readId();
	static long hex2dec(String hexCode);
};
#endif	// SeeedRFIDLib
