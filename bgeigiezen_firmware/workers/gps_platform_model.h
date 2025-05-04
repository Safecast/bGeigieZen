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

// UBX-CFG-NAV5 message constants
#define UBX_CLASS_CFG 0x06
#define UBX_ID_CFG_NAV5 0x24

#endif //BGEIGIEZEN_GPS_PLATFORM_MODEL_H_
