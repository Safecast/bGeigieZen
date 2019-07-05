
// Get around problem due to Arduino.h redefining min/max
// https://stackoverflow.com/questions/41093090/esp8266-error-macro-min-passed-3-arguments-but-takes-just-2<Paste>

#include <M5Stack.h>
#undef max
#undef min

#include "serial_source.hpp"
#include "threaded_gps.hpp"

struct GPSSerialIn : public SerialSource
{
  HardwareSerial *hs;
  GPSSerialIn(HardwareSerial *_hs) : hs(_hs) { }
  bool available() { return hs->available(); }
  char read() { return hs->read(); }
};

struct GPSSerialOut : public SerialSink
{
  HardwareSerial *hs;
  GPSSerialOut(HardwareSerial *_hs) : hs(_hs) { }
  size_t println(char *buf) { return hs->println(buf); };
  size_t write(char *buf) { return hs->write(buf); };
};

GPSSerialIn gps_serial_in(&Serial2);
GPSSerialOut gps_serial_out(&Serial);

// Receptacle for the GPS data
GPSThread gps_thread(&gps_serial_in, &gps_serial_out);
GPSData gps_data;

void setup() {

  //M5.begin();

  Serial.begin(115200);  // terminal

  // Starting the GPS
  Serial2.begin(9600);
  gps_thread.init();

  Serial.println("hello");
}

void loop() {
  // put your main code here, to run repeatedly:

  // Wait for new GPS data
  while (!gps_thread.prepare_data(gps_data))
    ;
  
  Serial.println(gps_data.nbsat);
  delay(1000);
  
}
