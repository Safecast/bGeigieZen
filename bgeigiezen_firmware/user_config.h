#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#define BGEIGIEZEN_VERSION "1.4"

/** System config **/
#define ENABLE_DEBUG 1

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