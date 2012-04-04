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
