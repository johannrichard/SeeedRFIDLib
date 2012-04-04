# SeeedRFIDLib, an Arduino Library for RFID Readers

This library allows you to interface your Arduino (Or compatible board) with a [SeeedStudio Electronic Brick 125kHz RFID Reader](http://www.seeedstudio.com/wiki/Electronic_brick_-_125Khz_RFID_Card_Reader). It might possibly work with other RFID Readers as well, if they use either an UART or Wiegand mode to transmit the Card ID information. The library will not work with RFID readers based on I2C or SPI. The library is released under the MIT license allowing all sorts of reuse.

## Use
Depending on ahardware switch, your RFID reader will either operate in UART mode or Wiegand mode. Depending on your selection, the library has to be initialized differently. 

For UART Mode, you use

    SeeedRFIDLib RFID(RFID_RX_PIN, RFID_TX_PIN);
    
For Wiegand Mode, you use
        
    SeeedRFIDLib RFID(WIEGAND_26BIT);
    
Both modes are explained in more detail below. Besides the difference in initialization, the library handles all the details transparently for you. Whenever a complete ID is available, the method ``RFID.isIdAvailable()`` will return ``true``, upon which you can get the tag via a call to ``RFID.readId()``. The value returned is in fact a simple struct with different member properties:

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

## UART Mode
UART Mode uses the [SoftwareSerial](http://arduino.cc/hu/Reference/SoftwareSerial) library to interface the Reader with the Arduino. By initializing an instance of the Class ``SeedRFIDLib`` with an RX and TX Pin (Typically, Pin 2 & 3 on the Arduino are a good choice), you can interface to the Board in UART Mode.

    #include <SoftwareSerial.h>
    #include <SeeedRFIDLib.h>

    // Connect the Reader's RX to the RX Pin and vice versa for TX
    #define RFID_RX_PIN 2
    #define RFID_TX_PIN 3

    // Configure the Library in UART Mode
    SeeedRFIDLib RFID(RFID_RX_PIN, RFID_TX_PIN);
    RFIDTag tag;

    void setup() {
      Serial.begin(57600);
      Serial.println("Serial Ready");
    }

    void loop() {  
      if(RFID.isIdAvailable()) {
        tag = RFID.readId();
        Serial.print("ID:       ");
        Serial.println(tag.id);
        Serial.print("ID (HEX): ");
        Serial.println(tag.raw);
      }
    }
    
## Wiegand Mode
The [Wiegand interface standard](http://en.wikipedia.org/wiki/Wiegand_interface) was widely used in card swipe readers in the 1980s. It's basically a very special procotol to transmit data bitwise.

    #include <SeeedRFIDLib.h>
    #include <SoftwareSerial.h> 
    // SoftwareSerial must be included because the library depends on it
    // If you use the Library in Wiegand Mode, then the compiler
    // will optimize the SoftwareSerial away

    // Configure the Library in Wiegand Mode
    // DATA0 of the RFID Reader must be connected 
    // to Pin 2 of the Arduino (INT0)
    // DATA1 of the RFID Reader must be connected
    // To Pin 3 of the Arduino (INT1)
    SeeedRFIDLib RFID(WIEGAND_26BIT); 
    RFIDTag tag;

    void setup() {
      Serial.begin(57600);
    }
    
    void loop() {
        if(RFID.isIdAvailable()) {
            tag = RFID.readId();
            // In Wiegand Mode, we only get the card code
            Serial.print("CC = ");
            Serial.println(tag.id); 
        }
    }

## Limitations
The following limitations apply to the RFIDLib, mainly because of the limitations of the underlying Hardware:

* No "Facility" code in Wiegand mode: In Wiegand mode, only the 3 bytes of the card's ID (often printed on the tag) are available from the reader.
* No writing: The SeedStudio RFID Reader does not have the capability to write to RFID tags so no write methods were added to the Library

## Notes
Part of the code inspired by [SeedStudio's Wiki page of the product](http://www.seeedstudio.com/wiki/Electronic_brick_-_125Khz_RFID_Card_Reader), parts from code by [PageMac](http://www.pagemac.com/source.php?f=azure/progs/arduino_hid_wiegand.ino). 