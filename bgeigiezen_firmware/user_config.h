#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#include <Arduino.h>

/** System config **/
#define ENABLE_DEBUG 1
#define STR(x) TOSTRING(x)
#define TOSTRING(x) #x

/*** VERSION ***/
// Version defined in platformio.ini in [env] build flags
#define VERSION_STRING STR(MAJOR_VERSION) "." STR(MINOR_VERSION) "." STR(PATCH_VERSION)

/***************/

/*** CONFIG ***/
// - geiger counter
constexpr uint8_t GEIGER_AVERAGING_N_BINS = 60;  // 60 x 1 s == 1 min moving average
constexpr float GEIGER_SENSOR1_CPM_FACTOR = 340.0;
constexpr uint8_t GEIGER_AVERAGING_PERIOD_S = 1;  // 1 s
#ifdef M5_CORE2
constexpr int GEIGER_PULSE_GPIO = 32;
// - RTC BM8563 on M5Stack Core2 I2C bus
constexpr uint8_t BM8563_I2C_SDA = 21;
constexpr uint8_t BM8563_I2C_SCL = 22;
constexpr uint8_t RTC_I2C_ADDRESS = 0x51;
#elif M5_BASIC
constexpr int GEIGER_PULSE_GPIO = 2;
#endif

// - GNSS
constexpr int GPS_SERIAL_NUM = 2;
constexpr uint32_t GPS_BAUD_RATE = 38400;
constexpr uint16_t GPS_INVALID_YEAR = 2000;
constexpr uint16_t GPS_PDOP_THRESHOLD = 500;  // Exact value TBD
constexpr uint8_t GPS_SATS_THRESHOLD = 4;  // strictly necessary is 4, but more is better
constexpr uint32_t GPS_FIX_AGE_LIMIT = 1500; // ms before we decide the fix is too old.

// - LCD display
constexpr uint32_t LCD_REFRESH_RATE = 1000;  // 1s

// - SD card
constexpr uint8_t SD_CS_PIN = 4;  // GPIO 4

// - URL for the QR code, the device number should be added at the end
constexpr char QR_CODE_URL_BASE[] = "http://tt.safecast.org/id/geigiecast:";
constexpr uint8_t QR_CODE_DEV_ID_NDIGITS = 4;

// - HEADER string for the log file
constexpr char LOG_HEADER_LINE1[] = "# NEW LOG";
constexpr char LOG_HEADER_LINE2[] = "# format=";
constexpr char LOG_HEADER_LINE3[] = "# deadtime=off";

// - Setup
constexpr char SETUP_FILENAME[] = "/SAFEZEN.txt";
constexpr uint8_t SETUP_FILE_PARSE_BUFFER_SIZE = 102;
constexpr uint32_t SETUP_DEFAULT_DEVICE_ID = 0;
constexpr uint8_t SETUP_USERNAME_MAXLEN = 15;
constexpr float SETUP_DEFAULT_USH_DIVIDER = 334.0;
constexpr float SETUP_DEFAULT_BQM2_FACTOR = 37.0;
constexpr uint32_t SETUP_DEFAULT_SENSOR_HEIGHT = 100;
constexpr uint8_t SETUP_DEFAULT_CPM_WINDOW = 60;
constexpr uint32_t SETUP_DEFAULT_ALERT_LEVEL = 100;
constexpr char SETUP_DEFAULT_COUNTRY_CODE[4] = "JPN";
constexpr uint8_t SETUP_DEFAULT_TIMEZONE = 9;

// - Logging
constexpr char DEVICE_HEADER[] = "BNRDD";
constexpr uint8_t LOG_BUFFER_SIZE = 100;

// Access point settings
constexpr char ACCESS_POINT_SSID[8] = "bgeigie"; // With device id"
constexpr uint32_t SERVER_WIFI_PORT = 80;

/**************/


/** Hardware pins settings **/
// TODO

/** API connector settings **/
#define API_HOST "tt.safecast.org"
#define API_MEASUREMENTS_ENDPOINT "http://" API_HOST "/measurements.json"
#define HEADER_API_CONTENT_TYPE "application/json"
#define HEADER_API_USER_AGENT "bGeigieCast/" VERSION_STRING

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