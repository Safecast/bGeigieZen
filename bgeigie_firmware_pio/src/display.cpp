#include <M5Stack.h>
#include <display.hpp>

void Display::setup() {
  // Clear display
  M5.Lcd.clear(); 
  M5.lcd.setRotation(3);

  //Display safecast copyright
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE,TFT_BLACK);
  M5.Lcd.drawString("2020", 285, 215, 1);

  // mark as ready
  _ready = true;
}

void Display::update(const GeigerMeasurement &geiger_count) {
  if (!_ready)
    return;

  if (geiger_count.valid())
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  else
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);

  //Display CPM  
  M5.Lcd.setCursor(20,50);
  M5.Lcd.drawString("CPM  =",5, 50, 4);
  M5.Lcd.setCursor(120,55);
  printIntFont(geiger_count.per_minute(), true, 5, 50, 90, 4);

  //Display uSv/h
  M5.Lcd.setCursor(22,70);
  M5.Lcd.drawString("uSv/h =",5, 70, 4);
  M5.Lcd.setCursor(100,80);
  printFloatFont(geiger_count.uSv(), true, 7, 3, 70, 90, 4);
}

void Display::update (GPSSensor &gps) {
  // Here we can't make the ref const because we need to access the fields
  // of the TinyGPS object directly...

  M5.Lcd.setCursor(0 , 150);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.print("Satelites  :");
  printInt(gps.data().satellites.value(), gps.data().satellites.isValid(), 5);
  M5.Lcd.println();
  M5.Lcd.print("Latitude   :");
  printFloat(gps.data().location.lat(), gps.data().location.isValid(), 11, 6);
  M5.Lcd.println();
  M5.Lcd.print("Longitude  :");
  printFloat(gps.data().location.lng(), gps.data().location.isValid(), 12, 6);
  M5.Lcd.println();
  M5.Lcd.print("Date       :");
  printDate(gps.data().date);
  M5.Lcd.println();
  M5.Lcd.print("Time       :");
  printTime(gps.data().time);
  M5.Lcd.println();
  M5.Lcd.print("Altitude   :");
  printFloat(gps.data().altitude.meters(), gps.data().altitude.isValid(), 7, 2);
  M5.Lcd.println();
  M5.Lcd.print("Degree     :");
  printFloat(gps.data().course.deg(), gps.data().course.isValid(), 7, 2);
  M5.Lcd.println();
  M5.Lcd.print("Speed      :");
  printFloat(gps.data().speed.kmph(), gps.data().speed.isValid(), 6, 2);
}

void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      M5.Lcd.print('*');
    M5.Lcd.print(' ');
  }
  else
  {
    M5.Lcd.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      M5.Lcd.print(' ');
  }
  delay(0);
}

void printFloatFont(float val, bool valid, int len, int prec, int y, int x, int font)
{
  if (!valid)
  {
    while (len-- > 1)
      M5.Lcd.print('*');
    M5.Lcd.print(' ');
  }
  else
  {
    char sz[32] = "*****************";
    if (valid)
      sprintf(sz, "%f", val);
    sz[len] = 0;
    for (int i=strlen(sz); i<len; ++i)
      sz[i] = ' ';
    if (len > 0) 
      sz[len-1] = ' ';
    M5.Lcd.drawString((sz),x, y, font);

    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      M5.Lcd.drawString("",x, y, font);
  }
  delay(0);
}

//Prints int 
void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  M5.Lcd.print(sz);
  delay(0);
}

//Prints int with fonts
void printIntFont(unsigned long val, bool valid, int len, int y, int x, int font)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  M5.Lcd.drawString((sz),x, y, font);
  delay(0);
}

//Prints date
void printDate(TinyGPSDate &d)
{
  if (!d.isValid())
  {
    M5.Lcd.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    M5.Lcd.print(sz);
  }
  printInt(d.age(), d.isValid(), 5);
  delay(0);
}


//Prints time
void printTime(TinyGPSTime &t)
{
  if (!t.isValid())
  {
    M5.Lcd.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    M5.Lcd.print(sz);
  }
  delay(0);
}
