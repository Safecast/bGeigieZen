// Test code from Average CPM count with sliding window for bGeigieRaku. Code taken from bGeigieNano and Pointcast.

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
/*
** IP5306 Power Module
*/

/* M5 Defaults
KeyOff: Enabled
BoostOutput: Disabled
PowerOnLoad: Enabled
Charger: Enabled
Boost: Enabled
LowBatShutdown: Enabled
ShortPressBoostSwitch: Disabled
FlashlightClicks: Double Press
BoostOffClicks: Long Press
BoostAfterVin: Open
LongPressTime: 2s
ChargeUnderVoltageLoop: 4.55V
ChargeCCLoop: Vin
VinCurrent: 2250mA
VoltagePressure: 28mV
ChargingFullStopVoltage: 4.17V
LightLoadShutdownTime: 32s
EndChargeCurrentDetection: 500mA
ChargeCutoffVoltage: 4.2V
*/

#define IP5306_REG_SYS_0    0x00
#define IP5306_REG_SYS_1    0x01
#define IP5306_REG_SYS_2    0x02
#define IP5306_REG_CHG_0    0x20
#define IP5306_REG_CHG_1    0x21
#define IP5306_REG_CHG_2    0x22
#define IP5306_REG_CHG_3    0x23
#define IP5306_REG_CHG_4    0x24
#define IP5306_REG_READ_0   0x70
#define IP5306_REG_READ_1   0x71
#define IP5306_REG_READ_2   0x72
#define IP5306_REG_READ_3   0x77
#define IP5306_REG_READ_4   0x78

#define IP5306_GetKeyOffEnabled()               ip5306_get_bits(IP5306_REG_SYS_0, 0, 1)
#define IP5306_SetKeyOffEnabled(v)              ip5306_set_bits(IP5306_REG_SYS_0, 0, 1, v)//0:dis,*1:en

#define IP5306_GetBoostOutputEnabled()          ip5306_get_bits(IP5306_REG_SYS_0, 1, 1)
#define IP5306_SetBoostOutputEnabled(v)         ip5306_set_bits(IP5306_REG_SYS_0, 1, 1, v)//*0:dis,1:en

#define IP5306_GetPowerOnLoadEnabled()          ip5306_get_bits(IP5306_REG_SYS_0, 2, 1)
#define IP5306_SetPowerOnLoadEnabled(v)         ip5306_set_bits(IP5306_REG_SYS_0, 2, 1, v)//0:dis,*1:en

#define IP5306_GetChargerEnabled()              ip5306_get_bits(IP5306_REG_SYS_0, 4, 1)
#define IP5306_SetChargerEnabled(v)             ip5306_set_bits(IP5306_REG_SYS_0, 4, 1, v)//0:dis,*1:en

#define IP5306_GetBoostEnabled()                ip5306_get_bits(IP5306_REG_SYS_0, 5, 1)
#define IP5306_SetBoostEnabled(v)               ip5306_set_bits(IP5306_REG_SYS_0, 5, 1, v)//0:dis,*1:en

#define IP5306_GetLowBatShutdownEnable()        ip5306_get_bits(IP5306_REG_SYS_1, 0, 1)
#define IP5306_SetLowBatShutdownEnable(v)       ip5306_set_bits(IP5306_REG_SYS_1, 0, 1, v)//0:dis,*1:en

#define IP5306_GetBoostAfterVin()               ip5306_get_bits(IP5306_REG_SYS_1, 2, 1)
#define IP5306_SetBoostAfterVin(v)              ip5306_set_bits(IP5306_REG_SYS_1, 2, 1, v)//0:Closed, *1:Open

#define IP5306_GetShortPressBoostSwitchEnable() ip5306_get_bits(IP5306_REG_SYS_1, 5, 1)
#define IP5306_SetShortPressBoostSwitchEnable(v) ip5306_set_bits(IP5306_REG_SYS_1, 5, 1, v)//*0:disabled, 1:enabled

#define IP5306_GetFlashlightClicks()            ip5306_get_bits(IP5306_REG_SYS_1, 6, 1)
#define IP5306_SetFlashlightClicks(v)           ip5306_set_bits(IP5306_REG_SYS_1, 6, 1, v)//*0:short press twice, 1:long press

#define IP5306_GetBoostOffClicks()              ip5306_get_bits(IP5306_REG_SYS_1, 7, 1)
#define IP5306_SetBoostOffClicks(v)             ip5306_set_bits(IP5306_REG_SYS_1, 7, 1, v)//*0:long press, 1:short press twice

#define IP5306_GetLightLoadShutdownTime()       ip5306_get_bits(IP5306_REG_SYS_2, 2, 2)
#define IP5306_SetLightLoadShutdownTime(v)      ip5306_set_bits(IP5306_REG_SYS_2, 2, 2, v)//0:8s, *1:32s, 2:16s, 3:64s

#define IP5306_GetLongPressTime()               ip5306_get_bits(IP5306_REG_SYS_2, 4, 1)
#define IP5306_SetLongPressTime(v)              ip5306_set_bits(IP5306_REG_SYS_2, 4, 1, v)//*0:2s, 1:3s

#define IP5306_GetChargingFullStopVoltage()     ip5306_get_bits(IP5306_REG_CHG_0, 0, 2)
#define IP5306_SetChargingFullStopVoltage(v)    ip5306_set_bits(IP5306_REG_CHG_0, 0, 2, v)//0:4.14V, *1:4.17V, 2:4.185V, 3:4.2V (values are for charge cutoff voltage 4.2V, 0 or 1 is recommended)

#define IP5306_GetChargeUnderVoltageLoop()      ip5306_get_bits(IP5306_REG_CHG_1, 2, 3)   //Automatically adjust the charging current when the voltage of VOUT is greater than the set value
#define IP5306_SetChargeUnderVoltageLoop(v)     ip5306_set_bits(IP5306_REG_CHG_1, 2, 3, v)//Vout=4.45V + (v * 0.05V) (default 4.55V) //When charging at the maximum current, the charge is less than the set value. Slowly reducing the charging current to maintain this voltage

#define IP5306_GetEndChargeCurrentDetection()   ip5306_get_bits(IP5306_REG_CHG_1, 6, 2)
#define IP5306_SetEndChargeCurrentDetection(v)  ip5306_set_bits(IP5306_REG_CHG_1, 6, 2, v)//0:200mA, 1:400mA, *2:500mA, 3:600mA

#define IP5306_GetVoltagePressure()             ip5306_get_bits(IP5306_REG_CHG_2, 0, 2)
#define IP5306_SetVoltagePressure(v)            ip5306_set_bits(IP5306_REG_CHG_2, 0, 2, v)//0:none, 1:14mV, *2:28mV, 3:42mV (28mV recommended for 4.2V)

#define IP5306_GetChargeCutoffVoltage()         ip5306_get_bits(IP5306_REG_CHG_2, 2, 2)
#define IP5306_SetChargeCutoffVoltage(v)        ip5306_set_bits(IP5306_REG_CHG_2, 2, 2, v)//*0:4.2V, 1:4.3V, 2:4.35V, 3:4.4V

#define IP5306_GetChargeCCLoop()                ip5306_get_bits(IP5306_REG_CHG_3, 5, 1)
#define IP5306_SetChargeCCLoop(v)               ip5306_set_bits(IP5306_REG_CHG_3, 5, 1, v)//0:BAT, *1:VIN

#define IP5306_GetVinCurrent()                  ip5306_get_bits(IP5306_REG_CHG_4, 0, 5)
#define IP5306_SetVinCurrent(v)                 ip5306_set_bits(IP5306_REG_CHG_4, 0, 5, v)//ImA=(v*100)+50 (default 2250mA)

#define IP5306_GetShortPressDetected()          ip5306_get_bits(IP5306_REG_READ_3, 0, 1)
#define IP5306_ClearShortPressDetected()        ip5306_set_bits(IP5306_REG_READ_3, 0, 1, 1)

#define IP5306_GetLongPressDetected()           ip5306_get_bits(IP5306_REG_READ_3, 1, 1)
#define IP5306_ClearLongPressDetected()         ip5306_set_bits(IP5306_REG_READ_3, 1, 1, 1)

#define IP5306_GetDoubleClickDetected()         ip5306_get_bits(IP5306_REG_READ_3, 2, 1)
#define IP5306_ClearDoubleClickDetected()       ip5306_set_bits(IP5306_REG_READ_3, 2, 1, 1)

#define IP5306_GetPowerSource()                 ip5306_get_bits(IP5306_REG_READ_0, 3, 1)//0:BAT, 1:VIN
#define IP5306_GetBatteryFull()                 ip5306_get_bits(IP5306_REG_READ_1, 3, 1)//0:CHG/DIS, 1:FULL
#define IP5306_GetOutputLoad()                  ip5306_get_bits(IP5306_REG_READ_2, 2, 1)//0:heavy, 1:light
#define IP5306_GetLevelLeds()                ((~ip5306_get_bits(IP5306_REG_READ_4, 4, 4)) & 0x0F)//LED[0-4] State (inverted)

#define IP5306_LEDS2PCT(byte)  \
  ((byte & 0x01 ? 25 : 0) + \
  (byte & 0x02 ? 25 : 0) + \
  (byte & 0x04 ? 25 : 0) + \
  (byte & 0x08 ? 25 : 0))

int ip5306_get_reg(uint8_t reg){
    Wire.beginTransmission(0x75);
    Wire.write(reg);
    if(Wire.endTransmission(false) == 0 && Wire.requestFrom(0x75, 1)){
        return Wire.read();
    }
    return -1;
}

int ip5306_set_reg(uint8_t reg, uint8_t value){
    Wire.beginTransmission(0x75);
    Wire.write(reg);
    Wire.write(value);
    if(Wire.endTransmission(true) == 0){
        return 0;
    }
    return -1;
}

uint8_t ip5306_get_bits(uint8_t reg, uint8_t index, uint8_t bits){
    int value = ip5306_get_reg(reg);
    if(value < 0){
        Serial.printf("ip5306_get_bits fail: 0x%02x\n", reg);
        return 0;
    }
    return (value >> index) & ((1 << bits)-1);
}

void ip5306_set_bits(uint8_t reg, uint8_t index, uint8_t bits, uint8_t value){
    uint8_t mask = (1 << bits) - 1;
    int v = ip5306_get_reg(reg);
    if(v < 0){
        Serial.printf("ip5306_get_reg fail: 0x%02x\n", reg);
        return;
    }
    v &= ~(mask << index);
    v |= ((value & mask) << index);
    if(ip5306_set_reg(reg, v)){
        Serial.printf("ip5306_set_bits fail: 0x%02x\n", reg);
    }
}



void printIP5306Stats(){
    bool usb = IP5306_GetPowerSource();
    bool full = IP5306_GetBatteryFull();
    uint8_t leds = IP5306_GetLevelLeds();
    Serial.printf("IP5306: Power Source: %s, Battery State: %s, Battery Available: %u%%\n", usb?"USB":"BATTERY", full?"CHARGED":(usb?"CHARGING":"DISCHARGING"), IP5306_LEDS2PCT(leds));
}

void printIP5306Settings(){
  delay (1009);
    Serial.println("IP5306 Settings:");
    Serial.printf("  KeyOff: %s\n", IP5306_GetKeyOffEnabled()?"Enabled":"Disabled");
    Serial.printf("  BoostOutput: %s\n", IP5306_GetBoostOutputEnabled()?"Enabled":"Disabled");
    Serial.printf("  PowerOnLoad: %s\n", IP5306_GetPowerOnLoadEnabled()?"Enabled":"Disabled");
    Serial.printf("  Charger: %s\n", IP5306_GetChargerEnabled()?"Enabled":"Disabled");
    Serial.printf("  Boost: %s\n", IP5306_GetBoostEnabled()?"Enabled":"Disabled");
    Serial.printf("  LowBatShutdown: %s\n", IP5306_GetLowBatShutdownEnable()?"Enabled":"Disabled");
    Serial.printf("  ShortPressBoostSwitch: %s\n", IP5306_GetShortPressBoostSwitchEnable()?"Enabled":"Disabled");
    Serial.printf("  FlashlightClicks: %s\n", IP5306_GetFlashlightClicks()?"Long Press":"Double Press");
    Serial.printf("  BoostOffClicks: %s\n", IP5306_GetBoostOffClicks()?"Double Press":"Long Press");
    Serial.printf("  BoostAfterVin: %s\n", IP5306_GetBoostAfterVin()?"Open":"Not Open");
    Serial.printf("  LongPressTime: %s\n", IP5306_GetLongPressTime()?"3s":"2s");
    Serial.printf("  ChargeUnderVoltageLoop: %.2fV\n", 4.45 + (IP5306_GetChargeUnderVoltageLoop() * 0.05));
    Serial.printf("  ChargeCCLoop: %s\n", IP5306_GetChargeCCLoop()?"Vin":"Bat");
    Serial.printf("  VinCurrent: %dmA\n", (IP5306_GetVinCurrent() * 100) + 50);
    Serial.printf("  VoltagePressure: %dmV\n", IP5306_GetVoltagePressure()*14);
    Serial.printf("  ChargingFullStopVoltage: %u\n", IP5306_GetChargingFullStopVoltage());
    Serial.printf("  LightLoadShutdownTime: %u\n", IP5306_GetLightLoadShutdownTime());
    Serial.printf("  EndChargeCurrentDetection: %u\n", IP5306_GetEndChargeCurrentDetection());
    Serial.printf("  ChargeCutoffVoltage: %u\n", IP5306_GetChargeCutoffVoltage());
    Serial.println();
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

  // send IP5003 info to serial
    printIP5306Settings();
    delay(1000);

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

  // send IP5003 data to serial
    printIP5306Stats();

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