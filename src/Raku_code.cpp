// Test code from Average CPM count with sliding window for bGeigeiRaku. Code taken from bGeigieNano and Pointcast.

#include <M5Stack.h>
#include <TinyGPS++.h>

//sliding windows counting variables
#define NX 60 // 60 times per minute buckets
#define TIME_INTERVAL 1000
#define IS_READY (interruptCounterAvailable())

// Stock font 
#define FF17 &FreeSans9pt7b
#define FF18 &FreeSans12pt7b
#define FF20 &FreeSans24pt7b

//int
int S1peak;

//floats
float conversionCoefficient ;
float sensor1_cpm_factor =340 ;

//long
  unsigned long elapsedTime(unsigned long startTime);
  unsigned long previousMillis = 0;
  unsigned long currentmillis = 0;
  unsigned long total_time = 0;

//long for sliding window
  unsigned long shift_reg[NX] = {0};
  unsigned long reg_index = 0;
  unsigned long total_count = 0;
  unsigned long max_count = 0;
  unsigned long uptime = 0;
  unsigned long _start_time;
  unsigned long _delay = TIME_INTERVAL;
  unsigned long _count = 0;
  unsigned long cpm = 0, cpb = 0;


// Sampling interval (e.g. 60,000ms = 1min)
  unsigned long updateIntervalInMillis = 0;
  unsigned long int counts_per_sample;


 
// The TinyGPS++ object
  TinyGPSPlus gps;

//Baud rate for serial of GPS
  static const uint32_t GPSBaud = 9600;

// The serial connection to the GPS device
  HardwareSerial ss(2);

//Compute CPM
  unsigned long cpm_gen()
    {
      unsigned int i;
      unsigned long c_p_m = 0;

      // sum up
      for (i = 0 ; i < NX ; i++)
        c_p_m += shift_reg[i];
      //deadtime compensation (medcom international)
      c_p_m = (unsigned long)((float)c_p_m / (1 - (((float)c_p_m * 1.8833e-6))));
      return c_p_m;
    }

//Pulse counter
  void onPulse()
  {
    counts_per_sample++;
  }

//sliding windows setup
  void interruptCounterReset()
  {
    // set start time
    _start_time = millis();
    // set count to zero (optional)
    counts_per_sample = 0;

  }

int interruptCounterAvailable()
{
  // get current time
  unsigned long now = millis();
  // do basic check for millis overflow
  if (now >= _start_time)
    return (now - _start_time >= _delay);
  else
    return (ULONG_MAX + now - _start_time >= _delay);
}

// return current number of counts
  unsigned long interruptCounterCount()
  {
    total_count=total_count+counts_per_sample;
    return counts_per_sample;
  }

// Displays battery level rough. Needs to be changed to bGeigieNano read LiPo fuelgauge.
  int8_t getBatteryLevel()
  {
    Wire.beginTransmission(0x75);
    Wire.write(0x78);
    if (Wire.endTransmission(false) == 0
    && Wire.requestFrom(0x75, 1)) {
      switch (Wire.read() & 0xF0) {
      case 0xE0: return 25;
      case 0xC0: return 50;
      case 0x80: return 75;
      case 0x00: return 100;
      default: return 3;
      }
    }
    return -1;
  }


// This custom version of delay() ensures that the gps object
// is being "fed".
  static void smartDelay(unsigned long ms)
  {
    unsigned long start = millis();
    do 
    {
      while (ss.available())
        gps.encode(ss.read());
    } while (millis() - start < ms);
  }

static void printFloat(float val, bool valid, int len, int prec)
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
    smartDelay(0);
  }

static void printFloatFont(float val, bool valid, int len, int prec, int y, int x, int font)
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
    smartDelay(0);
  }

//Prints int 
  static void printInt(unsigned long val, bool valid, int len)
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
    smartDelay(0);
  }

//Prints int with fonts
  static void printIntFont(unsigned long val, bool valid, int len, int y, int x, int font)
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
    smartDelay(0);
  }
//Prints date
  static void printDate(TinyGPSDate &d)
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
    smartDelay(0);
  }


//Prints time
  static void printTime(TinyGPSTime &t)
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
    smartDelay(0);
  }


//------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  //Serial setup for M5Stack
    M5.begin();
    ss.begin(GPSBaud);
    Serial.begin(115200);
    Serial.println("test");

  // SENSOR  setup for 
    conversionCoefficient = 1 / 340;
    attachInterrupt(2, onPulse, FALLING);

  // Start I2C communications for battery level indicator
    Wire.begin();

  // Clear display
    M5.Lcd.clear(); 
    M5.lcd.setRotation(3);

  //Beeps at startup
    M5.Speaker.tone(900, 1000);
    delay(10);
    M5.Speaker.mute();

  //Reset text padding to zero (default)
    // M5.Lcd.setTextPadding(0);
    // M5.Lcd.drawString("bGeigieRaku V0.5",20, 100, 4);

  //Display safecast copyright
    M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
    M5.Lcd.drawString("SAFECAST", 230, 215, 1);
    M5.Lcd.setTextColor(TFT_ORANGE,TFT_BLACK);
    M5.Lcd.drawString("2019", 285, 215, 1);
  }

//------------------------------------------------------------------------------------------------------------------------------------------

void loop()
{

// Display battery level
    M5.Lcd.setCursor(290, 3);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    if (getBatteryLevel()==-1) {
          M5.Lcd.print("ext");
    }else{
          M5.Lcd.print(getBatteryLevel());
          M5.Lcd.print("%");
    }
//Display GPS data
    M5.Lcd.setCursor(0 , 150);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.print("Satelites  :");
    printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
    M5.Lcd.println();
    M5.Lcd.print("Latitude   :");
    printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    M5.Lcd.println();
    M5.Lcd.print("Longitude  :");
    printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
    M5.Lcd.println();
    M5.Lcd.print("Date       :");
    printDate(gps.date);
    M5.Lcd.println();
    M5.Lcd.print("Time       :");
    printTime(gps.time);
    M5.Lcd.println();
    M5.Lcd.print("Altitude   :");
    printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
    M5.Lcd.println();
    M5.Lcd.print("Degree     :");
    printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
    M5.Lcd.println();
    M5.Lcd.print("Speed      :");
    printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);

    M5.Lcd.println();
    
    smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    M5.Lcd.println(F("No GPS data received: check wiring"));

  // Send reading data if IS_READY is passed when 
  if IS_READY {

    cpb = interruptCounterCount();
      interruptCounterReset();

      // insert count in sliding window and compute CPM
      shift_reg[reg_index] = cpb;     // put the count in the correct bin
      reg_index = (reg_index + 1) % NX; // increment register index
      cpm = cpm_gen();               // compute sum over all bins
    
    
    // Convert CPM to uSvH
      conversionCoefficient = 1 / sensor1_cpm_factor;
      float uSv = cpm * conversionCoefficient;                   // convert CPM to Micro Sievers Per Hour
      char CPM_string[16];
      dtostrf(cpm, 0, 0, CPM_string);
      if (cpm > (int)(S1peak)) {
        S1peak = cpm;
      }

          M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      //Display CPM  
          M5.Lcd.setCursor(20,50);
          M5.Lcd.drawString("CPM  =",5, 50, 4);
          M5.Lcd.setCursor(120,55);
          // printInt(cpm, true, 5);
          printIntFont(cpm, true, 5, 50, 90, 4);

      //Display uSv/h
          M5.Lcd.setCursor(22,70);
          M5.Lcd.drawString("uSv/h =",5, 70, 4);
          M5.Lcd.setCursor(100,80);
          printFloatFont(uSv, true, 7, 3, 70, 90, 4);

      //prints data
        Serial.print("cpb (60 bins) =");
        Serial.println(cpb);
        Serial.print("cpm =");
        Serial.println(cpm);
        Serial.print("uSvH =");
        Serial.println(uSv);    
        Serial.print("S1peak =");
        Serial.println(S1peak); 
        Serial.print("Total counts =");
        Serial.println(total_count);
        Serial.println();
    }
}