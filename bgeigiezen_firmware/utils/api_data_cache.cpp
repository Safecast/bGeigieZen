#include "api_data_cache.h"

ApiDataCache& ApiDataCache::instance() {
  static ApiDataCache cache;
  return cache;
}

void ApiDataCache::update(const GeigerData& g)    { _geiger  = g; }
void ApiDataCache::update(const GnssData& g)      { _gnss    = g; }
void ApiDataCache::update(const BatteryStatus& b) { _battery = b; }

// ---------------------------------------------------------------------------
// JSON helpers — append a key:value pair to a String (no external library)
// ---------------------------------------------------------------------------
static void aj_float(String& s, const char* k, float v, int dec = 4) {
  s += '"'; s += k; s += "\":"; s += String(v, dec);
}
static void aj_double(String& s, const char* k, double v, int dec = 6) {
  s += '"'; s += k; s += "\":"; s += String(v, dec);
}
static void aj_uint(String& s, const char* k, uint32_t v) {
  s += '"'; s += k; s += "\":"; s += v;
}
static void aj_int(String& s, const char* k, int32_t v) {
  s += '"'; s += k; s += "\":"; s += v;
}
static void aj_bool(String& s, const char* k, bool v) {
  s += '"'; s += k; s += "\":"; s += v ? "true" : "false";
}
static void aj_str(String& s, const char* k, const char* v) {
  s += '"'; s += k; s += "\":\""; s += v; s += '"';
}

// ---------------------------------------------------------------------------

String ApiDataCache::toJson(uint16_t device_id,
                            const char* firmware,
                            const char* mode) const {
  // Build ISO-8601 timestamp from GPS or leave default
  char ts[24] = "1970-01-01T00:00:00Z";
  if (_gnss.date_valid && _gnss.time_valid) {
    snprintf(ts, sizeof(ts), "%04u-%02u-%02uT%02u:%02u:%02uZ",
             _gnss.year, _gnss.month, _gnss.day,
             _gnss.hour, _gnss.minute, _gnss.second);
  }

  // Charging state as a string
  const char* charging_str = "unknown";
  switch (_battery.isCharging) {
    case m5::Power_Class::is_charging_t::is_charging:    charging_str = "charging";      break;
    case m5::Power_Class::is_charging_t::is_discharging: charging_str = "discharging";   break;
    case m5::Power_Class::is_charging_t::charge_unknown: charging_str = "unknown";       break;
  }

  String j;
  j.reserve(600);
  j = "{";

  // device
  j += "\"device\":{";
  aj_uint(j, "id", device_id);          j += ',';
  aj_str (j, "firmware", firmware);     j += ',';
  aj_str (j, "mode", mode);
  j += "},";

  // radiation
  j += "\"radiation\":{";
  aj_uint (j, "cpm",       _geiger.cpm_comp);       j += ',';
  aj_uint (j, "cpm_raw",   _geiger.cpm_raw);         j += ',';
  aj_float(j, "uSvh",      _geiger.uSvh);            j += ',';
  aj_float(j, "uSvh_5sec", _geiger.uSvh_5sec);       j += ',';
  aj_float(j, "Bqm2",      _geiger.Bqm2,      2);    j += ',';
  aj_bool (j, "valid",     _geiger.valid);            j += ',';
  aj_bool (j, "alert",     _geiger.alert);
  j += "},";

  // gps — speed: gSpeed is mm/s, convert to km/h (* 0.0036)
  j += "\"gps\":{";
  aj_bool  (j, "valid",           _gnss.valid());                           j += ',';
  aj_double(j, "latitude",        _gnss.latitude);                          j += ',';
  aj_double(j, "longitude",       _gnss.longitude);                         j += ',';
  aj_float (j, "altitude_m",      (float)_gnss.altitudeMSL,          1);   j += ',';
  aj_float (j, "speed_kmh",       (float)_gnss.gSpeed * 0.0036f,     2);   j += ',';
  aj_float (j, "heading_deg",     (float)_gnss.heading_degree,        1);   j += ',';
  aj_uint  (j, "satellites_used", _gnss.numSV);                             j += ',';
  aj_uint  (j, "satellites_view", _gnss.satsInView);                        j += ',';
  aj_float (j, "pdop",            (float)_gnss.pdop,                  1);
  j += "},";

  // battery
  j += "\"battery\":{";
  aj_int  (j, "pct",        _battery.percentage);        j += ',';
  aj_float(j, "voltage_v",  _battery.voltage,       2);  j += ',';
  aj_float(j, "current_ma", _battery.current_mA,    1);  j += ',';
  aj_str  (j, "charging",   charging_str);
  j += "},";

  // timestamp & uptime
  j += "\"timestamp\":\""; j += ts; j += "\",";
  aj_uint(j, "uptime_s", millis() / 1000);

  j += '}';
  return j;
}
