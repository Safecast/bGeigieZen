#include <M5Stack.h>

HardwareSerial GPSRaw(2);

void setup() {

  M5.begin();
  GPSRaw.begin(9600);

  Serial.begin(115200);
  Serial.println("hello");
}

void loop() {
  // put your main code here, to run repeatedly:

  if(GPSRaw.available()) {
    char ch = GPSRaw.read();
    Serial.print(ch);
  }
}
