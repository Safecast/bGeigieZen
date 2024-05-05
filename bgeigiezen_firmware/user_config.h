#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

/** System config **/
#define ENABLE_DEBUG 1
#define STR(x) TOSTRING(x)
#define TOSTRING(x) #x

/*** VERSION ***/
// Version defined in platformio.ini in [env] build flags
#define VERSION_NUMBER STR(MAJOR_VERSION) "." STR(MINOR_VERSION) "." STR(PATCH_VERSION)
#define VERSION_STRING "bGeigieZen " VERSION_NUMBER

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
constexpr uint32_t GPS_FIX_AGE_LIMIT = 4500; // ms before we decide the fix is too old.

// - LCD display
constexpr uint32_t LCD_REFRESH_RATE = 1000;  // 1s
constexpr uint32_t STATUS_MESSAGE_DURATION = 5000;  // 5s
#define LCD_COLOR_BACKGROUND TFT_BLACK
#define LCD_COLOR_DEFAULT TFT_WHITE
#define LCD_COLOR_STALE_INCOMPLETE TFT_ORANGE
#define LCD_COLOR_ERROR TFT_RED
#define LCD_COLOR_INACTIVE TFT_DARKGREY
#define LCD_COLOR_ACTIVITY TFT_GREEN

// - SD card
constexpr uint8_t SD_CS_PIN = 4;  // GPIO 4

// - Setup
constexpr char SETUP_FILENAME[] = "/SAFEZEN.txt";
constexpr char TEST_FILENAME[] = "/.zen";
constexpr float SETUP_DEFAULT_USH_DIVIDER = 334.0;
constexpr float SETUP_DEFAULT_BQM2_FACTOR = 37.0;
constexpr uint32_t SETUP_DEFAULT_ALERT_LEVEL = 100;

// - Logging
constexpr char JOURNAL_LOG_DIRECTORY[] = "/journals";
constexpr char DRIVE_LOG_DIRECTORY[] = "/drives";
constexpr char SURVEY_LOG_DIRECTORY[] = "/surveys";
constexpr char LOG_HEADER_LINE1[] = "# NEW LOG";
constexpr char LOG_HEADER_LINE2[] = "# format=";
constexpr char LOG_HEADER_LINE3[] = "# deadtime=off";
constexpr char DEVICE_HEADER[] = "BNRDD";
constexpr uint8_t LINE_BUFFER_SIZE = 100;
constexpr uint8_t LOG_FILENAME_SIZE = 100;
constexpr uint8_t MIN_LOG_LINES_FOR_KEEP = 2;
constexpr uint8_t LOG_SECONDS_DELAY = 5; // Logs every 5 seconds
constexpr uint16_t API_SEND_SECONDS_DELAY = 60; // Posts every 1 minute by default
constexpr uint16_t API_SEND_SECONDS_DELAY_ALERT = 5; // Posts every 5 seconds when alerted
constexpr uint16_t API_SEND_SECONDS_DELAY_ROAMING = 5; // Posts every 5 seconds when roaming

constexpr char FIXED_MODE_GRAFANA_URL[] = "https://grafana.safecast.cc/d/DFSxrOLWk/safecast-device-details?from=now-24h&to=now&refresh=5m&var-device_urn=geigiecast:%d";

// Other
constexpr char SCREENSAVER_TEXT[] = VERSION_STRING;


/**************/


/** Hardware pins settings **/
// TODO

/** API connector settings **/
#define API_HOST "tt.safecast.org"
#define API_MEASUREMENTS_ENDPOINT "http://" API_HOST "/measurements.json"
#define HEADER_API_CONTENT_TYPE "application/json"
#define HEADER_API_USER_AGENT "bGeigieZen/" VERSION_NUMBER

/** Access point settings **/
#define ACCESS_POINT_SSID       "bgeigiezen-%d" // With device id
#define SERVER_WIFI_PORT        80
#define ACCESS_POINT_IP         {192, 168, 5, 1}
#define ACCESS_POINT_NMASK      {255, 255, 255, 0}

/** Default configurations **/
#define D_DEVICE_ID 0
#define D_AP_PASSWORD "safecast"
#define D_ALARM_THRESHOLD 100
#define D_CPM_USVH false
#define D_MANUAL_LOGGING false
#define D_ENABLE_JOURNAL true
#define D_LOG_VOID false
#define D_SCREEN_DIM_TIMEOUT 60
#define D_SCREEN_OFF_TIMEOUT 600
#define D_ANIMATED_SCREENSAVER true
#define D_WIFI_SSID "your wifi ssid"
#define D_WIFI_PASSWORD "your wifi password"
#define D_API_KEY ""
#define D_FIXED_LONGITUDE 0
#define D_FIXED_LATITUDE 0
#define D_FIXED_RANGE 0.5
#define D_LAST_LONGITUDE 0
#define D_LAST_LATITUDE 0


#endif