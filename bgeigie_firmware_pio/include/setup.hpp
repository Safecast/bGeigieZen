/*
   Simple library to handle nano configuration from file and EEPROM
   Copyright (c) 2012, Lionel Bergeret
   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __SETUP_HPP__
#define __SETUP_HPP__

// Link to arduino library
#include <cstring>
#include <M5Core2.h> // #include <M5Stack.h>
#include <config.hpp>
#include <Preferences.h>

const char SETUP_NAMESPACE[] = "bgzen-data";
const char SETUP_KEY_CONFIG[] = "config";
const char SETUP_KEY_DOSE[] = "dose";

// geiger dose
// write every hours (eeprom ~ 100000 cycles -> ~ 11 years)
const uint32_t BNRDD_EEPROM_DOSE_WRITETIME = 3600;

enum GeigieType { GEIGIE_TYPE_B = 0, GEIGIE_TYPE_X };

enum GeigieMode { GEIGIE_MODE_USVH = 0, GEIGIE_MODE_BQM2 };

enum SensorType {
  SENSOR_TYPE_LND7317 = 0,
  SENSOR_TYPE_LND712,
};

enum SensorShield {
  SENSOR_SHIELD_NONE = 0,
  SENSOR_SHIELD_ALPHA,
  SENSOR_SHIELD_ALPHABETA,
};

enum SensorMode {
  SENSOR_MODE_AIR = 0,
  SENSOR_MODE_SURFACE = 1,
  SENSOR_MODE_PLANE = 2
};

struct DoseData {
  uint32_t total_count = 0;
  uint32_t total_time = 0;
};

struct ConfigData {
  ConfigData() { strcpy(country_code, SETUP_DEFAULT_COUNTRY_CODE); }
  uint8_t type = GEIGIE_TYPE_B;     // 0 for bGeigie, 1 for xGeigie
  uint8_t mode = GEIGIE_MODE_USVH;  // 0 for uSv/h, 1 for Bq/m2
  char user_name[SETUP_USERNAME_MAXLEN + 1] = {0};
  uint32_t device_id = SETUP_DEFAULT_DEVICE_ID;
  uint8_t cpm_window = SETUP_DEFAULT_CPM_WINDOW;
  float cpm2ush_divider = SETUP_DEFAULT_USH_DIVIDER;
  float cpm2bqm2_factor = SETUP_DEFAULT_BQM2_FACTOR;
  uint32_t alarm_level = SETUP_DEFAULT_ALARM_LEVEL;  // in CPM
  uint8_t timezone = SETUP_DEFAULT_TIMEZONE;         // in hours
  char country_code[sizeof(SETUP_DEFAULT_COUNTRY_CODE)];
  uint8_t sensor_type = SENSOR_TYPE_LND7317;
  uint8_t sensor_shield = SENSOR_SHIELD_NONE;
  uint32_t sensor_height = SETUP_DEFAULT_SENSOR_HEIGHT;  // in cm
  uint8_t sensor_mode = SENSOR_MODE_AIR;

  void print() const {
    Serial.print("Type: "); Serial.println(type);
    Serial.print("Mode: "); Serial.println(mode);
    Serial.print("Username: "); Serial.println(user_name);
    Serial.print("Device_id: "); Serial.println(device_id);
    Serial.print("CPM window: "); Serial.println(cpm_window);
    Serial.print("CPM factor: "); Serial.println(cpm2ush_divider);
    Serial.print("BQM2 factor: "); Serial.println(cpm2bqm2_factor);
    Serial.print("Alarm level: "); Serial.println(alarm_level);
    Serial.print("Timezone: "); Serial.println(timezone);
    Serial.print("Country code: "); Serial.println(country_code);
    Serial.print("Sensor type: "); Serial.println(sensor_type);
    Serial.print("Sensor shield: "); Serial.println(sensor_shield);
    Serial.print("Sensor height: "); Serial.println(sensor_height);
    Serial.print("Sensor mode: "); Serial.println(sensor_mode);
  }
};

class Setup {
 public:
  Setup(){};
  void initialize();
  bool load_from_file(const char *setup_filename);
  bool ready() const { return _ready; }
  const ConfigData &config() const { return _config; }
  const DoseData &dose() const { return _dose; }

 private:
  bool _ready = false;
  Preferences _prefs;  // persistent storage connection
  ConfigData _config;   // configuration data
  DoseData _dose;       // dose data
};

#endif  // __SETUP_HPP__
