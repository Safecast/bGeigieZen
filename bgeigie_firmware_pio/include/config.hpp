#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

/*** VERSION ***/
const uint16_t MAJOR = 0;
const uint16_t MINOR = 1;
const uint16_t PATCH = 0;
const char device_nickname[] = "zen";
/***************/

/*** CONFIG ***/
// TEMPORARY UNTIL WE GET CONFIG GOING
const uint32_t DEVICE_ID = 1;
// - log format header
const char DEVICE_HEADER[] = "BZRDD";
// - maximum length of buffer for the bGeigie log
const size_t LOG_BUFFER_SIZE = 100;
// - geiger counter
const size_t GEIGER_AVERAGING_PERIOD_MS = 5000;  // 5 s
const int GEIGER_PULSE_GPIO = 2;
const size_t GEIGER_AVERAGING_N_BINS = 12;  // 12 x 5 s == 1 min moving average
const float GEIGER_SENSOR1_CPM_FACTOR = 340.0;
// - gps
const int GPS_SERIAL_NUM = 2;
const uint32_t GPS_BAUD_RATE = 9600;
const uint16_t GPS_INVALID_YEAR = 2000;
// - LCD display
const uint32_t LCD_REFRESH_RATE = 1000;  // 1s
// - SD card
const uint8_t SD_CS_PIN = 4;  // GPIO 4
// - URL for the QR code, the device number should be added at the end
const char QR_CODE_URL_BASE[] = "http://tt.safecast.org/id/geigiecast:";
// - HEADER string for the log file
const char LOG_HEADER_LINE1[] = "# NEW LOG";
const char LOG_HEADER_LINE2[] = "# format=";
const char LOG_HEADER_LINE3[] = "# deadtime=off";

/**************/

#endif  // __CONFIG_HPP__
