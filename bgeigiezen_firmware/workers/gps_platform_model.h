#ifndef BGEIGIEZEN_GPS_PLATFORM_MODEL_H_
#define BGEIGIEZEN_GPS_PLATFORM_MODEL_H_

// u-blox Dynamic Platform Models
enum UbxDynamicModel {
  DYNMODEL_PORTABLE = 0,    // Portable (default)
  DYNMODEL_STATIONARY = 2,  // Stationary
  DYNMODEL_PEDESTRIAN = 3,  // Pedestrian
  DYNMODEL_AUTOMOTIVE = 4,  // Automotive
  DYNMODEL_SEA = 5,         // Sea
  DYNMODEL_AIRBORNE_1G = 6, // Airborne with <1g acceleration
  DYNMODEL_AIRBORNE_2G = 7, // Airborne with <2g acceleration
  DYNMODEL_AIRBORNE_4G = 8, // Airborne with <4g acceleration (AIR4)
  DYNMODEL_WRIST = 9        // Wrist-worn (not all receivers)
};

// Aliases for clarity in code
#define DYNMODEL_PORT DYNMODEL_PORTABLE
#define DYNMODEL_AIR4 DYNMODEL_AIRBORNE_4G

// UBX message constants
#define UBX_SYNC_CHAR_1 0xB5
#define UBX_SYNC_CHAR_2 0x62

// UBX-CFG message constants
#define UBX_CLASS_CFG 0x06
#define UBX_ID_CFG_NAV5 0x24
#define UBX_CFG_VALSET 0x8A
#define UBX_CFG_VALGET 0x8B

// UBX-CFG-VALSET layers
#define UBX_CFG_LAYER_RAM 0x01
#define UBX_CFG_LAYER_BBR 0x02
#define UBX_CFG_LAYER_FLASH 0x04
#define UBX_CFG_LAYER_ALL (UBX_CFG_LAYER_RAM | UBX_CFG_LAYER_BBR | UBX_CFG_LAYER_FLASH)

// UBX-CFG-NAVSPG-DYNMODEL key ID
#define UBX_CFG_NAVSPG_DYNMODEL 0x20110021

#endif //BGEIGIEZEN_GPS_PLATFORM_MODEL_H_
