/*****************************************
* ESP32 GPS VKEL 9600 Bds
******************************************/

#include <TinyGPS++.h>   

//SDCard init
#include <mySD.h> // https://github.com/LilyGO/esp32-micro-sdcard

//Preference storage init init
#include <Preferences.h>


/* create an instance of Preferences library */
Preferences preferences;

TinyGPSPlus gps;                            
HardwareSerial Serial1(1);  


//LED init
#define LED_BUILTIN 22  //Blink pin 

//Buttons init
//#define Button1 38  
#define Button2 37
#define Button3 39


 //Variables ------------------------------------------
const uint32_t partial_update_period_s = 1;
const uint32_t full_update_period_s = 1 * 60 * 60;

uint32_t start_time;
uint32_t next_time;
uint32_t previous_time;
uint32_t previous_full_update;
uint32_t total_seconds = 0;
uint32_t seconds, minutes, hours, days;

uint32_t data;

int ledState = LOW;  
const int ledPin =  LED_BUILTIN;

const byte interruptPin1 = 38;//button 1
boolean buttonState1;

unsigned long previousMillis = 0; 
const long interval = 10; 

//SDcard setup file folder
File root;              

//Buttons init
//  unsigned int buttonState1 = 0; 

//Display init
#include <GxEPD.h>

#include <GxGDEH029A1/GxGDEH029A1.cpp>      // 2.9" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSevenSegNumFont.h>

GxIO_Class io(SPI, SS, 17, 16);
GxEPD_Class display(io, 16, 4);


//saves button
void store_button(){
//      preferences.begin("iotsharing", false);
//      preferences.putBool("buttonState1", buttonState1);
         Serial.printf("Button 1 state:");
         Serial.printf(buttonState1 ? "true" : "false");
         Serial.println();
      /* Close the Preferences */
//      preferences.end();
}

// Runs from interrupt pin 38
void activate_button1(){
      buttonState1=!buttonState1;
      store_button();
}

void read_write() {
  //Prefences read/wite test:   
          preferences.begin("iotsharing", false);
      
        // Remove all preferences under the opened namespace
      //   preferences.clear();
        
         unsigned int reset_times = preferences.getUInt("reset_times", 0);
         bool buttonState1 = preferences.getBool("buttonState1", 0);
         
         reset_times++;
         buttonState1=!buttonState1;
         
         Serial.println();
         Serial.printf("Number of restart times: %d\n", reset_times);
         Serial.printf("Button 1 state:");
         Serial.printf(buttonState1 ? "true" : "false");
         Serial.println();
//         Serial.printf("Button 1 state: %\n", buttonState1);
         preferences.putUInt("reset_times", reset_times);
         preferences.putBool("buttonState1", buttonState1);
         preferences.end();
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 33, 32);
  
  //Setup buttons interrupts 
  pinMode(interruptPin1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin1), activate_button1, HIGH);
    
  //Display setup
  Serial.println("setup display ");
  display.init(); // disable diagnostic output on Serial
  Serial.println("setup done");
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.update(); //Not sure if needed to clear the buffers
  display.update();
  display.setFont(&FreeMono9pt7b);

  start_time = next_time = previous_time = previous_full_update = millis();
  display.setRotation(1);

  //SD card setup
  Serial.println("setup SDCARD");
//  pinMode(ledPin, OUTPUT);
   
  Serial.print("Initializing SD card...");  
  if (!SD.begin(13,15,2,14)) { /* initialize SD library with SPI pins */
    Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");
  /* open "test.txt" for writing */
  root = SD.open("test1.txt", FILE_WRITE);
  if (!root) {
    Serial.println("error opening test.txt");
  }
  // SD card write test
  if (root) {
    root.println("Hello world!");
    root.flush();
   /* close the file */
    root.close();
  } else {
    /* if the file open error, print an error */
    Serial.println("error opening test.txt");
  }
  
  delay(1000);
  /* after writing then reopen the file and read it */
  root = SD.open("test1.txt");
  if (root) {    
    while (root.available()) {    /* read from the file until there's nothing else in it */
      Serial.write(root.read());  /* read the file and print to Terminal */
    }
    root.close();
    Serial.println();
  } else {
    Serial.println("error opening test.txt");
  }
  Serial.println("SD Card setup done!");

  read_write();
}



static void smartDelay(unsigned long ms)                
  {
    unsigned long start = millis();
    do
    {
      while (Serial1.available())
        gps.encode(Serial1.read());
    } while (millis() - start < ms);
  }
 
void print02d(uint32_t d)
{
  if (d < 10) display.print("0");
  display.print(d);
}

void showGPS()
  {

    uint16_t box_x = 5;
    uint16_t box_y = 5;
    uint16_t box_w = 400;
    uint16_t box_h = 122;
    uint16_t cursor_y = box_y + 16;
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y);
    display.setFont(&FreeMonoBold9pt7b);
    display.println(" bGeigieNano E-Paper V0.9");
    display.setFont(&FreeMono9pt7b);
    display.print(" Latitude  : ");
    display.println(gps.location.lat(), 5);
    display.print(" Longitude : ");
    display.println(gps.location.lng(), 4);
    display.print(" Satellites: ");
    display.println(gps.satellites.value());
    display.print(" Altitude  : ");
    display.print(gps.altitude.feet() / 3.2808);
    display.println("M");
    display.print(" Time      : "); print02d(gps.time.hour()); display.print(":"); print02d(gps.time.minute()); display.print(":"); print02d(gps.time.second());
    display.updateWindow(box_x, box_y, box_w, box_h, true);
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  }

 void showData()
  {
    uint16_t box_x = 5;
    uint16_t box_y = 5;
    uint16_t box_w = 400;
    uint16_t box_h = 122;
    uint16_t cursor_y = box_y + 48;
    display.setFont(&FreeSevenSegNumFont);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y);
    display.print(data);
    display.setFont(&FreeMonoBold9pt7b);
    display.println(" CPM");
    display.println("");
    display.println("");
    display.setFont(&FreeMonoBold9pt7b);
    display.println("  bGeigieNano E-paper V0.9 ");
    display.println("              SAFECAST 2018");    
    display.updateWindow(box_x, box_y, box_w, box_h, true);
 
  }

void loop()
{
  uint32_t actual = millis();
      while (Serial1.available())
        gps.encode(Serial1.read());
        
  data = random(30, 37);

 // read the state of the pushbutton value:

  if (buttonState1) {
   showData();
   
  } else {
   showGPS(); 
  }
}
