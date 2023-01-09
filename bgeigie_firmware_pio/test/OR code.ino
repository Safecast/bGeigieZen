#include <M5Core2.h> // #include <M5Stack.h>

void setup() {
  M5.begin();
  M5.Power.begin();
  // Display QRCode
  M5.Lcd.qrcode("https://grafana.safecast.cc/d/DFSxrOLWk/safecast-device-details?orgId=1&from=now-7d&to=now&refresh=15m&var-device_urn=geigiecast:61377");
  // M5.Lcd.qrcode(const char *string, uint16_t x = 50, uint16_t y = 10, uint8_t width = 220, uint8_t version = 6);
}

void loop() {
}