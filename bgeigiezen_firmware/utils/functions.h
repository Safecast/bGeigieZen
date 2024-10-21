//
// Created by klei on 10/7/24.
//

#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_FUNCTIONS_H_
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_FUNCTIONS_H_

#include "M5Unified.h"
#define D2R (PI / 180.0)

/**
 * Calculate distance using haversine formula
 * @param lat1
 * @param long1
 * @param lat2
 * @param long2
 * @return distance in km
 */
inline double haversine_km(double lat1, double long1, double lat2, double long2) {
  double dlong = (long2 - long1) * D2R;
  double dlat = (lat2 - lat1) * D2R;
  double a = pow(sin(dlat / 2.0), 2) + cos(lat1 * D2R) * cos(lat2 * D2R) * pow(sin(dlong / 2.0), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return 6367 * c;
}

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_UTILS_FUNCTIONS_H_
