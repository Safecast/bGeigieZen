#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <stdint.h>

#define BGEIGIEZEN_VERSION "1.0"

/** System config **/
#define ENABLE_DEBUG 1


/*** VERSION ***/
const uint16_t MAJOR = 0;
const uint16_t MINOR = 1;
const uint16_t PATCH = 0;
/***************/

/*** CONFIG ***/
// - log format header
const char DEVICE_HEADER[] = "BNRDD";
// - maximum length of buffer for the bGeigie log
const uint8_t LOG_BUFFER_SIZE = 100;
// - geiger counter
const uint8_t GEIGER_AVERAGING_PERIOD_S = 5;  // 5 s
const int GEIGER_PULSE_GPIO = 32;
const uint8_t GEIGER_AVERAGING_N_BINS = 12;  // 12 x 5 s == 1 min moving average
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
const uint8_t QR_CODE_DEV_ID_NDIGITS = 4;
// - HEADER string for the log file
const char LOG_HEADER_LINE1[] = "# NEW LOG";
const char LOG_HEADER_LINE2[] = "# format=";
const char LOG_HEADER_LINE3[] = "# deadtime=off";
// - Setup
const char SETUP_FILENAME[] = "/SAFECAST.txt";
const uint8_t SETUP_FILE_PARSE_BUFFER_SIZE = 102;
const uint32_t SETUP_DEFAULT_DEVICE_ID = 0;
const uint8_t SETUP_USERNAME_MAXLEN = 15;
const float SETUP_DEFAULT_USH_DIVIDER = 334.0;
const float SETUP_DEFAULT_BQM2_FACTOR = 37.0;
const uint32_t SETUP_DEFAULT_SENSOR_HEIGHT = 100;
const uint8_t SETUP_DEFAULT_CPM_WINDOW = 60;
const uint32_t SETUP_DEFAULT_ALARM_LEVEL = 100;
const char SETUP_DEFAULT_COUNTRY_CODE[4] = "JPN";
const uint8_t SETUP_DEFAULT_TIMEZONE = 9;
// Access point settings
const char ACCESS_POINT_SSID[8] = "bgeigie"; // With device id"
const uint32_t SERVER_WIFI_PORT = 80;

/**************/


/** Hardware pins settings **/
// TODO

/** API connector settings **/
#define API_HOST "tt.safecast.org"
#define HEADER_API_CONTENT_TYPE "application/json"
#define HEADER_API_USER_AGENT "bGeigieCast/" BGEIGIEZEN_VERSION
#define API_MEASUREMENTS_ENDPOINT "http://" API_HOST "/measurements.json"

/** Access point settings **/
#define ACCESS_POINT_SSID       "bgeigiezen-%d" // With device id
#define SERVER_WIFI_PORT        80
#define ACCESS_POINT_IP         {192, 168, 5, 1}
#define ACCESS_POINT_NMASK      {255, 255, 255, 0}

/** Default configurations **/
#define D_DEVICE_ID             0
#define D_ACCESS_POINT_PASSWORD "safecast"
#define D_WIFI_SSID             "your wifi ssid"
#define D_WIFI_PASSWORD         "yourwifipassword"
#define D_APIKEY                ""
#define D_USE_DEV_SERVER        true

#endif