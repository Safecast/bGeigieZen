// Test code from Average CPM count with sliding window for bGeigeiRaku. Code taken from bGeigieNano and Pointcast.

#include <M5Stack.h>
#include <hardwarecounter.hpp>
#include <display.hpp>

// Peripherals
HardwareCounter pulse_counter(5000, 2);  // pulse counter with 12 bin moving average

const float sensor1_cpm_factor = 340;
GeigerMeasurement<12> geiger_count(sensor1_cpm_factor);

void setup()
{
  //Serial setup for M5Stack
  M5.begin();
  Serial.begin(115200);
  Serial.println("test");

  // Clear display
  M5.Lcd.clear(); 
  M5.lcd.setRotation(3);

  //Beeps at startup
  // M5.Speaker.tone(900, 1000);
  // delay(10);
  // M5.Speaker.mute();

  //Reset text padding to zero (default)
  // M5.Lcd.setTextPadding(0);
  // M5.Lcd.drawString("bGeigieRaku V0.5",20, 100, 4);

  //Display safecast copyright
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE,TFT_BLACK);
  M5.Lcd.drawString("2020", 285, 215, 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void loop()
{

  pulse_counter.update();

  // Send reading data if IS_READY is passed when 
  if (pulse_counter.available()) {

    // update the Geiger counter with the new measurement
    geiger_count.feed(pulse_counter.get_last_count());

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    //Display CPM  
    M5.Lcd.setCursor(20,50);
    M5.Lcd.drawString("CPM  =",5, 50, 4);
    M5.Lcd.setCursor(120,55);
    // printInt(cpm, true, 5);
    printIntFont(geiger_count.per_minute(), true, 5, 50, 90, 4);

    //Display uSv/h
    M5.Lcd.setCursor(22,70);
    M5.Lcd.drawString("uSv/h =",5, 70, 4);
    M5.Lcd.setCursor(100,80);
    printFloatFont(geiger_count.uSv(), true, 7, 3, 70, 90, 4);

    //prints data
    Serial.print("cpb (60 bins) =");
    Serial.println(geiger_count.per_bin());
    Serial.print("cpm =");
    Serial.println(geiger_count.per_minute());
    Serial.print("uSvH =");
    Serial.println(geiger_count.uSv());    
    Serial.print("S1peak =");
    Serial.println(geiger_count.peak_per_minute()); 
    Serial.print("Total counts =");
    Serial.println(geiger_count.total());
    Serial.println();
  }
}
