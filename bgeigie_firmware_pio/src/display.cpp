#include <M5Core2.h> // #include <M5Stack.h>


#include <cstdio>
#include <display.hpp>

void Display::clear() {
  // Clear display
  M5.Lcd.clear();
  M5.lcd.setRotation(3);
}

void Display::draw_base() {
  // Display safecast copyright
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Lcd.drawString("2023", 285, 215, 1);
}

void Display::draw_navbar(const char *A, const char *B, const char *C) {
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.drawString(A, 50, 10, 2); // Button A
  M5.Lcd.drawString(B, 145, 10, 2); // Button B
  M5.Lcd.drawString(C, 250, 10, 2); // Button C
}

void Display::update() {
  // This function is mainly used to read the buttons and update the state
  // of the display

  bool button_A_pressed = M5.BtnA.wasPressed();
  bool button_B_pressed = M5.BtnB.wasPressed();
  bool button_C_pressed = M5.BtnC.wasPressed();

  switch (state) {
    case (S_STARTUP):
      clear();  // initialize display
      draw_base();
      state = S_MAIN_DRAW;  // next state
      break;

    case (S_MAIN_DRAW):
      clear();
      draw_base();
      state = S_MAIN_SHOW;
    case (S_MAIN_SHOW):
      draw_main();
      if (button_A_pressed) state = S_QRCODE_DRAW;
      break;

    case (S_QRCODE_DRAW):
      clear();
      draw_qrcode();
      state = S_QRCODE_SHOW;
      break;

    case (S_QRCODE_SHOW):
      if (button_A_pressed) state = S_MAIN_DRAW;
      break;
  }
}

void Display::draw_qrcode() {
  // Create the QR code
  int w = (int)(0.9 * height);
  if (w % 2 == 1) w -= 1;
  int x = (width - w) / 2;
  int y = (height - w) / 2;

  // create the URL
  char url[sizeof(QR_CODE_URL_BASE) + QR_CODE_DEV_ID_NDIGITS];
  std::sprintf(url, "%s%04d", QR_CODE_URL_BASE, (int)data.device_id);

  M5.Lcd.qrcode(url, x, y, w, 3);
};

void Display::draw_main() {

  draw_navbar("MENU", "MODE", "QR");

  // Show the device number
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      M5.Lcd.print("DeviceID =");
  M5.Lcd.print(data.device_id);

  // Display battery level
  M5.Lcd.setCursor(270, 30);
  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  if (data.battery_level == -1) {
    M5.Lcd.print("BAT=ext");
  } else {
    M5.Lcd.print("BAT=");
    M5.Lcd.print(data.battery_level);
    M5.Lcd.print("%");
  }

  if (data.geiger_fresh) {
    if (data.geiger_valid)
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    else
      M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);

    // Display CPM
    M5.Lcd.setCursor(20, 50);
    M5.Lcd.drawString("CPM ", 120, 70, 4);
    M5.Lcd.setCursor(120, 55);
    printIntFont(data.geiger_cpm, true, 5, 50, 5, 6);

    // Display uSv/h
    M5.Lcd.setCursor(22, 70);
    M5.Lcd.drawString("uSv/h =", 5, 90, 4);
    M5.Lcd.setCursor(100, 80);
    printFloatFont(data.geiger_uSv, true, 7, 3, 90, 90, 4);

    data.geiger_fresh = false;
  }

  if (data.gps_fresh) {
    M5.Lcd.setCursor(0, 150);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.print("Satelites  :");
    printInt(data.gps_satellites.value(), data.gps_satellites.isValid(), 5);
    M5.Lcd.println();
    M5.Lcd.print("Latitude   :");
    printFloat(data.gps_location.lat(), data.gps_location.isValid(), 11, 6);
    M5.Lcd.println();
    M5.Lcd.print("Longitude  :");
    printFloat(data.gps_location.lng(), data.gps_location.isValid(), 12, 6);
    M5.Lcd.println();
    M5.Lcd.print("Date       :");
    printDate(data.gps_date);
    M5.Lcd.println();
    M5.Lcd.print("Time       :");
    printTime(data.gps_time);
    M5.Lcd.println();
    M5.Lcd.print("Altitude   :");
    printFloat(data.gps_altitude.meters(), data.gps_altitude.isValid(), 7, 2);
    M5.Lcd.println();
    M5.Lcd.print("Degree     :");
    printFloat(data.gps_course.deg(), data.gps_course.isValid(), 7, 2);
    M5.Lcd.println();
    M5.Lcd.print("Speed      :");
    printFloat(data.gps_speed.kmph(), data.gps_speed.isValid(), 6, 2);

    data.gps_fresh = false;
  }
}

void Display::feed(const GeigerCounter &geiger_count) {
  data.geiger_valid = geiger_count.valid();
  data.geiger_cpm = geiger_count.per_minute();
  data.geiger_uSv = geiger_count.uSv();
  data.geiger_fresh = true;
}

void Display::feed(GPSSensor &gps) {
  // Here we can't make the ref const because we need to access the fields
  // of the TinyGPS object directly...

  data.gps_satellites = gps.data().satellites;
  data.gps_speed = gps.data().speed;
  data.gps_course = gps.data().course;
  data.gps_location = gps.data().location;
  data.gps_altitude = gps.data().altitude;
  data.gps_time = gps.data().time;
  data.gps_date = gps.data().date;
  data.gps_fresh = true;
}

void printFloat(float val, bool valid, int len, int prec) {
  if (!valid) {
    while (len-- > 1) M5.Lcd.print('*');
    M5.Lcd.print(' ');
  } else {
    M5.Lcd.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);  // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i) M5.Lcd.print(' ');
  }
  delay(0);
}

void printFloatFont(float val, bool valid, int len, int prec, int y, int x,
                    int font) {
  if (!valid) {
    while (len-- > 1) M5.Lcd.print('*');
    M5.Lcd.print(' ');
  } else {
    char sz[32] = "*****************";
    if (valid) sprintf(sz, "%f", val);
    sz[len] = 0;
    for (int i = strlen(sz); i < len; ++i) sz[i] = ' ';
    if (len > 0) sz[len - 1] = ' ';
    M5.Lcd.drawString((sz), x, y, font);

    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);  // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i) M5.Lcd.drawString("", x, y, font);
  }
  delay(0);
}

// Prints int
void printInt(unsigned long val, bool valid, int len) {
  char sz[32] = "*****************";
  if (valid) sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i) sz[i] = ' ';
  if (len > 0) sz[len - 1] = ' ';
  M5.Lcd.print(sz);
  delay(0);
}

// Prints int with fonts
void printIntFont(unsigned long val, bool valid, int len, int y, int x,
                  int font) {
  char sz[32] = "*****************";
  if (valid) sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i) sz[i] = ' ';
  if (len > 0) sz[len - 1] = ' ';
  M5.Lcd.drawString((sz), x, y, font);
  delay(0);
}

// Prints date
void printDate(TinyGPSDate &d) {
  if (!d.isValid()) {
    M5.Lcd.print(F("********** "));
  } else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.year(), d.month(), d.day());
    M5.Lcd.print(sz);
  }
  printInt(d.age(), d.isValid(), 5);  // age in milliseconds
  delay(0);
}

// Prints time
void printTime(TinyGPSTime &t) {
  if (!t.isValid()) {
    M5.Lcd.print(F("******** "));
  } else {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    M5.Lcd.print(sz);
  }
  delay(0);
}
