/*
   Simple library to handle nano configuration from file and NVS memory
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

#include <config.hpp>
#include <cstdlib>
#include <cstring>
#include <setup.hpp>

// The states for the FSM used to parse the config file
enum LineParseState {
  LINE_START,
  LINE_PARSE_KEY,
  LINE_PARSE_DELIM,
  LINE_PARSE_VALUE,
  LINE_SKIP
};

void Setup::initialize() {
  // Configuration
  // 2nd arg (false) indicates that we want Read/Write operation
  _prefs.begin(SETUP_NAMESPACE, false);

  if (_prefs.getBytesLength(SETUP_KEY_CONFIG) == 0) {
    // This is the first time we set up the device
    _prefs.putBytes(SETUP_KEY_CONFIG, &_config, sizeof(ConfigData));
  } else {
    _prefs.getBytes(SETUP_KEY_CONFIG, &_config, sizeof(ConfigData));
  }

  if (_prefs.getBytesLength(SETUP_KEY_DOSE) == 0) {
    // This is the first time we set up the device
    _prefs.putBytes(SETUP_KEY_DOSE, &_dose, sizeof(DoseData));
  } else {
    _prefs.getBytes(SETUP_KEY_DOSE, &_dose, sizeof(DoseData));
  }

  _ready = true;
}

bool Setup::load_from_file(const char *setup_filename) {
  bool config_changed = false;
  char config_buffer[SETUP_FILE_PARSE_BUFFER_SIZE];
  char *key = config_buffer;
  char *value = config_buffer;

  // open the setup file
  auto setup_file = SD.open(setup_filename, FILE_READ);
  
  if (!setup_file)
    return false;

  // Process each config file lines
  char c;
  uint32_t pos = 0;
  LineParseState pstate = LINE_START;
  bool parsed_line = false;

  while (true) {
    c = (char)setup_file.read();
    if ((int8_t)c == -1)
      break;

    switch (pstate) {
      case LINE_START:
        if (c != ' ' && c != '\n' && c != '\r') {
          // found beginning of key
          config_buffer[pos] = c;
          pos++;  // pos == 1 after this line
          pstate = LINE_PARSE_KEY;
        }
        break;

      case LINE_PARSE_KEY:
        if (pos == SETUP_FILE_PARSE_BUFFER_SIZE) {
          pstate = LINE_SKIP;
        } else if (c == ' ') {
          // found end of the key, mark and go to the value
          config_buffer[pos] = '\0';  // mark the end of the string
          pos++;
          pstate = LINE_PARSE_DELIM;
        } else if (c == '=') {
          config_buffer[pos] = '\0';
          pos++;
          pstate = LINE_PARSE_DELIM;
        } else if (c == '\n' || c == '\r') {
          // premature end of line, skip
          pos = 0;
          pstate = LINE_START;
        } else {
          config_buffer[pos] = c;
          pos++;
        }
        break;

      case LINE_PARSE_DELIM:
        if (pos == SETUP_FILE_PARSE_BUFFER_SIZE) {
          pstate = LINE_SKIP;
        } else if (c == '\n' || c == '\r') {
          // we also accept an empty value
          value = config_buffer + pos;
          config_buffer[pos] = '\0';
          pos++;
          parsed_line = true;  // mark parsing as successful
          // parse next line
          pos = 0;
          pstate = LINE_START;
        } else if (c != ' ' && c != '=') {
          value = config_buffer + pos;
          config_buffer[pos] = c;
          pos++;
          pstate = LINE_PARSE_VALUE;
        }
        break;

      case LINE_PARSE_VALUE:
        if (pos == SETUP_FILE_PARSE_BUFFER_SIZE) {
          pstate = LINE_SKIP;
        } else if (c == '\n' || c == '\r' || c == ' ') {
          // end of value
          config_buffer[pos] = '\0';
          parsed_line = true;  // indicate successful parsing
          pos = 0;
          pstate = LINE_START;
        } else {
          config_buffer[pos] = c;
          pos++;
        }
        break;

      case LINE_SKIP:
        // wait until we find the end of line
        if (c == '\n' || c == '\r') {
          pos = 0;
          pstate = LINE_START;
        }
        break;
    }

    if (parsed_line) {
      parsed_line = false;

      //
      // Process matching keys
      //
      if (std::strcmp(key, "cpmf") == 0) {
        // Update cpm factor
        float factor = std::atof(value);
        if (_config.cpm2ush_divider != factor && factor > 0.0) {
          _config.cpm2ush_divider = factor;
          config_changed = true;
        }
      } else if (std::strcmp(key, "bqmf") == 0) {
        // Update bq/m2 factor
        float factor = std::atof(value);
        if (_config.cpm2bqm2_factor != factor && factor > 0.0) {
          _config.cpm2bqm2_factor = factor;
          config_changed = true;
        }
      } else if (std::strcmp(key, "nm") == 0) {
        // Update name in EEPROM
        if (std::strcmp(_config.user_name, value) != 0) {
          std::strncpy(_config.user_name, value, SETUP_USERNAME_MAXLEN);
          config_changed = true;
        }
      } else if (std::strcmp(key, "did") == 0) {
        // Update device id in persistent storage
        if (_config.device_id != (unsigned int)std::atoi(value)) {
          _config.device_id = std::atoi(value);
          config_changed = true;
        }
      } else if (std::strcmp(key, "gt") == 0) {
        // Update geiger type in persistent storage
        if (_config.type != std::atoi(value)) {
          _config.type = std::atoi(value);
          config_changed = true;
        }
      } else if (std::strcmp(key, "gm") == 0) {
        // Update geiger mode in persistent storage
        if (_config.mode != std::atoi(value)) {
          _config.mode = std::atoi(value);
          config_changed = true;
        }
      } else if (std::strcmp(key, "al") == 0) {
        // Update alarm threshold in persistent storage
        uint32_t new_alarm_level = std::atoi(value);
        if (_config.alarm_level != new_alarm_level) {
          _config.alarm_level = new_alarm_level;
          config_changed = true;
        }
      } else if (std::strcmp(key, "cn") == 0) {
        // Update country code in persistent storage
        if (std::strcmp(_config.country_code, value) != 0) {
          std::strncpy(_config.country_code, value,
                       sizeof(SETUP_DEFAULT_COUNTRY_CODE) - 1);
          config_changed = true;
        }
      } else if (std::strcmp(key, "tz") == 0) {
        // Update timezone in persistent storage
        uint8_t new_timezone = std::atoi(value);
        if (_config.timezone != new_timezone) {
          _config.timezone = new_timezone;
          config_changed = true;
        }
      } else if (std::strcmp(key, "st") == 0) {
        // Update sensor type in persistent storage
        uint8_t new_type = std::atoi(value);
        if (_config.sensor_type != new_type) {
          _config.sensor_type = new_type;
          config_changed = true;
        }
      } else if (std::strcmp(key, "ss") == 0) {
        // Update sensor shield in persistent storage
        uint8_t new_sensor_shield = std::atoi(value);
        if (_config.sensor_shield != new_sensor_shield) {
          _config.sensor_shield = new_sensor_shield;
          config_changed = true;
        }
      } else if (std::strcmp(key, "sh") == 0) {
        // Update sensor height in persistent storage
        uint32_t new_sensor_height = std::atoi(value);
        if (_config.sensor_height != new_sensor_height) {
          _config.sensor_height = new_sensor_height;
          config_changed = true;
        }
      } else if (std::strcmp(key, "sm") == 0) {
        // Update sensor mode in persistent storage
        if (_config.sensor_mode != std::atoi(value)) {
          _config.sensor_mode = std::atoi(value);
          config_changed = true;
        }
      } else if (std::strcmp(key, "dose") == 0) {
        // Reset total dose in persistent storage
        std::memset(&_dose, 0, sizeof(DoseData));
        _prefs.putBytes(SETUP_KEY_DOSE, &_dose, sizeof(DoseData));
      }
    }
  }

  // close the setup file
  setup_file.close();

  if (config_changed) {
    // Configuration is changed
    _prefs.putBytes(SETUP_KEY_CONFIG, &_config, sizeof(ConfigData));
  }

  return true;
}
