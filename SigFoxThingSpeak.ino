/*
  SigFox Simple Weather Station
  by Giovanni Gentile
  0lab.it
  August 2017
  
  Send to ThingSepeak:
  status
  internal temp
  DHT-11 temp
  DHT-11 humi
*/

#include <ArduinoLowPower.h>
#include <SigFox.h>
#include <DHT.h>

#define DHTPIN 1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float temperature;
float t;
float h;

/*
    ATTENTION - the structure we are going to send MUST
    be declared "packed" otherwise we'll get padding mismatch
    on the sent data - see http://www.catb.org/esr/structure-packing/#_structure_alignment_and_padding
    for more details
*/
typedef struct __attribute__ ((packed)) sigfox_message {
  uint8_t status;
  int16_t moduleTemperature;
  int16_t t;
  int16_t h;
} SigfoxMessage;

// stub for message which will be sent
SigfoxMessage msg;

void setup() {
  if (!SigFox.begin()) {
    // Something is really wrong, try rebooting
    // Reboot is useful if we are powering the board using an unreliable power source
    // (eg. solar panels or other energy harvesting methods)
    reboot();
  }
  
  //Send module to standby until we need to send a message
  SigFox.end();
  
  SigFox.debug();
  
  dht.begin();
}


void loop() {
  
  t = dht.readTemperature();
  msg.t = convertoFloatToInt16(t, 60, -60);
  
  h = dht.readHumidity();
  msg.h = convertoFloatToUInt16(h, 110);
  
  // Start the module
  SigFox.begin();
  // Wait at least 30ms after first configuration (100ms before)
  delay(100);

  // We can only read the module temperature before SigFox.end()
  temperature = SigFox.internalTemperature();
  msg.moduleTemperature = convertoFloatToInt16(temperature, 60, -60);
  
  
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg, 12);

  int lastMessageStatus = SigFox.endPacket();

  SigFox.end();

  //Sleep for 15 minutes
  LowPower.sleep(15 * 60 * 1000);
}
void reboot() {
   NVIC_SystemReset();
   while (1) ;
}
