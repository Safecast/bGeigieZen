/** @brief GNSS handler using UBX protocol library 
 * 
 * When activated, set auto receive for the UBX protocol commands used:
 * UBX-NAV-PVT: position, velocity and time
 * ***REMOVED*** UBX-NAV-DOP: dilution of precision, for horizontal DOP
 * ***REMOVED*** UBX-NAV-SAT: enumeration of satellites in view, for number of sats.
 * Based on examples by Paul Clark, SparkFun Electronics
 * https://github.com/sparkfun/SparkFun_u-blox_GNSS_Arduino_Library
 * 
 * ***REMOVED*** The two removed packets were not providing any additional
 * useful information. HDOP was always zero. NSATS was the total number of
 * satellites, regardless of their status in computing a fix.
*/

#include "utils/functions.h"
#include "gps_connector.h"
#include "utils/api_data_cache.h"
#include <SD.h>

#define GPS_INVALID_YEAR 2000
#define GPS_INVALID_MONTH 1
#define GPS_INVALID_DAY 1
#define GPS_INVALID_HOUR 0
#define GPS_INVALID_MINUTE 0
#define GPS_INVALID_SECOND 0

#define NNE 22.5
#define ENE (NNE + 45)
#define ESE (ENE + 45)
#define SSE (ESE + 45)
#define SSW (SSE + 45)
#define WSW (SSW + 45)
#define WNW (WSW + 45)
#define NNW (WNW + 45)

GpsConnector::GpsConnector(TeenyUbloxConnect& gnss, HardwareSerial& serial) : Worker<GnssData>({
                                                       .location_valid=false,
                                                       .date_valid=false,
                                                       .time_valid=false,
                                                       .latitude=0,
                                                       .longitude=0,
                                                       .altitudeMSL=0,
                                                       .heading_degree=0,
                                                       .heading=GnssData::UNKNOWN,
                                                       .satsInView=0,
                                                       .pdop=0,
                                                       .year=0,
                                                       .month=0,
                                                       .day=0,
                                                       .hour=0,
                                                       .minute=0,
                                                       .second=0,
                                                       .protocolVersionHigh=0,
                                                       .protocolVersionLow=0,
                                                       .nmea_sats={},
                                                       .nmea_sat_count=0,
                                                       .nmea_gsv_cycle=0,
                                                       .nmea_mode=false
                                                   }), _gnss(gnss), _serial_conn(serial), _tried_115200_at(0), _tried_38400_at(0), _tried_9600_at(0), _tried_4800_at(0), _init_at(0), _raw_dump_done(false), _nmea_mode(false), _nmea_len(0), _nmea_used_count(0), _last_latitude(0), _last_longitude(0) {
}
/**
 * @return true if initialized GNSS library, false if no connection with module.
*/
bool GpsConnector::activate(bool retry) {
  // From Sparkfun examples/Example12_UseUart
  // Assume that the U-Blox GNSS is running at 9600 baud (the default) or at 38400 baud.
  if (!retry) {
#ifdef CONFIG_IDF_TARGET_ESP32S3 // Core S3SE fix using the correct serial pins
    _serial_conn.begin(38400, SERIAL_8N1, RXD2, TXD2);
#else
    _serial_conn.begin(38400);
#endif

    _init_at = millis();
  }
  if (_tried_115200_at == 0 && (millis() - _init_at > 1000)) { // Wait for device to completely startup
    _tried_115200_at = millis();
    _serial_conn.updateBaudRate(115200);
    M5_LOGD("GNSS: Try at 115200 baud");
    if (_gnss.begin(_serial_conn, 500)) {
      M5_LOGD("GNSS: connected at 115200 baud"); // no need to set module to 38.4
    }
    else {
      return false;
    }
  }
  else if (_tried_38400_at == 0 && _tried_115200_at > 0 && (millis() - _tried_115200_at > 1200)) { // Wait for device to completely startup
    _tried_38400_at = millis();
    _serial_conn.updateBaudRate(38400);
    M5_LOGD("GNSS: Try at 38400 baud");
    if (_gnss.begin(_serial_conn, 500)) {
      M5_LOGD("GNSS: connected at 38400 baud"); // no need to set module to 38.4
    }
    else {
      return false;
    }
  }
  else if (_tried_9600_at == 0 && _tried_38400_at > 0 && (millis() - _tried_38400_at > 1200)) {
    _tried_9600_at = millis();
    _serial_conn.updateBaudRate(9600);
    M5_LOGD("GNSS: Try at 9600 baud");
    if (_gnss.begin(_serial_conn, 1000)) {
      M5_LOGD("GNSS: connected at 9600 baud, switching to 38400");
      _gnss.setSerialRate(38400);
      delay(100); // recovery time for gnss module baud rate change
      _serial_conn.updateBaudRate(38400);
    }
    else {
      return false;
    }
  }
  else if (_tried_4800_at == 0 && _tried_9600_at > 0 && (millis() - _tried_9600_at > 1200)) {
    _tried_4800_at = millis();
    _serial_conn.updateBaudRate(4800);
    M5_LOGD("GNSS: Try at 4800 baud");
    if (_gnss.begin(_serial_conn, 1000)) {
      M5_LOGD("GNSS: connected at 4800 baud, switching to 38400");
      _gnss.setSerialRate(38400);
      delay(100);
      _serial_conn.updateBaudRate(38400);
    }
    else {
      return false;
    }
  }
  else if (!_raw_dump_done && _tried_4800_at > 0 && (millis() - _tried_4800_at > 1200)) {
    // All UBX baud-rate attempts failed — test TX line and check for UBX response
    _raw_dump_done = true;
    _serial_conn.updateBaudRate(9600);
    while (_serial_conn.available()) _serial_conn.read(); // flush RX
    // Send a raw UBX-CFG-PRT poll (class 0x06, id 0x06, payload = UART1 port id 0x01)
    // B5 62 06 06 01 00 01 0E 24
    const uint8_t ubxPoll[] = {0xB5, 0x62, 0x06, 0x06, 0x01, 0x00, 0x01, 0x0E, 0x24};
    _serial_conn.write(ubxPoll, sizeof(ubxPoll));
    _serial_conn.flush();
    delay(300); // wait for GPS to respond
    uint8_t buf[128];
    int n = 0;
    uint32_t t = millis();
    while (n < (int)sizeof(buf) && (millis() - t) < 300) {
      if (_serial_conn.available()) buf[n++] = _serial_conn.read();
    }
    if (n > 0) {
      bool hasUbx = false;
      bool hasNmea = false;
      for (int i = 0; i < n; i++) {
        if (i < n - 1 && buf[i] == 0xB5 && buf[i+1] == 0x62) hasUbx = true;
        if (buf[i] == '$') hasNmea = true;
      }
      if (hasNmea && !hasUbx) {
        M5_LOGI("GNSS: GPS module: NMEA-only (TX not connected) — u-blox or compatible, 9600 baud");
        // Serial is already at 9600 baud on RXD2/TXD2 — just flush and switch mode.
        // Re-calling end()/begin() corrupts the UART on ESP32S3.
        delay(100);
        while (_serial_conn.available()) _serial_conn.read(); // flush stale bytes
        _nmea_mode = true;
        data.nmea_mode = true;
        data.nmea_sat_count = 0;
        memset(_nmea_buf, 0, sizeof(_nmea_buf));
        _nmea_len = 0;
        _nmea_used_count = 0;
        return true;
      }
    } else {
      M5_LOGW("GNSS: No bytes at 9600 — check wiring RX=GPIO18, TX=GPIO17 on CoreS3");
    }
    return false;
  }
  else {
    return false;
  }

  // Confirm that we actually have a connection
  uint8_t pvHigh = _gnss.getProtocolVersionHigh();
  uint8_t pvLow  = _gnss.getProtocolVersionLow();
  data.protocolVersionHigh = pvHigh;
  data.protocolVersionLow = pvLow;
  const char* gnssModuleName = pvHigh < 18 ? "M7" : pvHigh < 32 ? "M8" : pvHigh < 34 ? "M9" : "M10";
  M5_LOGI("GNSS: GPS module: u-blox %s, protocol v%02d.%02d", gnssModuleName, pvHigh, pvLow);
              
  // Check the current dynamic model on startup
  // This will help us verify if the setting persisted after power cycling
  M5_LOGD("GNSS: Reading current dynamic model...");
  readDynamicModelFromGPS();

  // Send UBX, disable NMEA-0183 messages that we are ignoring anyway.
  _gnss.setPortOutput(COM_PORT_UART1, COM_TYPE_UBX);

  // Set Auto on NAV-PVT for non-blocking access
  // getPVT() will return true if a new navigation solution is available
  _gnss.setAutoNAVPVT(true); // Tell the GNSS to send the solution as it is computed (1 second)
  _gnss.setAutoNAVSAT(false); // Disable navsat by default (navsat worker handles this)

  // Announce/perform restore path on boot (relies on internal backup domain)
  restoreGpsMemoryFromNVS();

  // Try to restore full database from SD if available
  restoreDatabaseFromSD();

  // Try to inject warm-start seed from SD if present
  injectWarmStartSeedFromSD();

  return true;
}

// Simple warm seed format
// magic 0x47574D53 ('GWMS'), version 1, lat/lon in 1e-7 deg, alt mm, date/time
struct WarmSeedBinV1 {
  uint32_t magic;     // 'GWMS'
  uint16_t version;   // 1
  int32_t lat_e7;
  int32_t lon_e7;
  int32_t alt_mm;     // altitude MSL in mm
  uint16_t year;
  uint8_t month, day, hour, minute, second;
};

// Age limits in seconds
static constexpr uint32_t kSeedMaxAge = 7 * 24 * 3600; // 7 days
static constexpr uint32_t kDbdMaxAge = 12 * 3600; // 12 hours

static constexpr uint32_t kSeedMagic = 0x47574D53; // GWMS
static constexpr uint16_t kSeedVer = 1;
static constexpr const char* kSeedDir = "/gnss";
static constexpr const char* kSeedPath = "/gnss/warm_seed.bin";
static constexpr const char* kDbdPath = "/gnss/dbd_latest.bin";

// Forward declarations
class GpsConnector;
static bool sendMgaIniTimeUtc(GpsConnector* gps, uint16_t year, uint8_t month, uint8_t day, 
                              uint8_t hour, uint8_t minute, uint8_t second);
static bool sendMgaIniPosLlh(GpsConnector* gps, int32_t lat_1e7, int32_t lon_1e7, int32_t alt_mm);

bool GpsConnector::saveWarmStartSeedToSD() {
  // Ensure we have valid data
  if (!(data.location_valid && data.date_valid && data.time_valid)) {
    M5_LOGW("GPS: Cannot save warm seed — invalid or incomplete GNSS data");
    return false;
  }

  // Ensure directory exists
  if (!SD.exists(kSeedDir)) {
    if (!SD.mkdir(kSeedDir)) {
      M5_LOGE("GPS: Failed to create %s directory on SD", kSeedDir);
      return false;
    }
  }

  WarmSeedBinV1 seed{};
  seed.magic = kSeedMagic;
  seed.version = kSeedVer;
  seed.lat_e7 = static_cast<int32_t>(data.latitude * 1e7);
  seed.lon_e7 = static_cast<int32_t>(data.longitude * 1e7);
  seed.alt_mm = static_cast<int32_t>(data.altitudeMSL * 1000.0);
  seed.year = data.year;
  seed.month = data.month;
  seed.day = data.day;
  seed.hour = data.hour;
  seed.minute = data.minute;
  seed.second = data.second;

  File f = SD.open(kSeedPath, FILE_WRITE);
  if (!f) {
    M5_LOGE("GPS: Failed to open %s for write", kSeedPath);
    return false;
  }
  size_t written = f.write(reinterpret_cast<const uint8_t*>(&seed), sizeof(seed));
  f.flush();
  f.close();
  if (written != sizeof(seed)) {
    M5_LOGE("GPS: Failed to write full seed (wrote %u/%u bytes)", (unsigned)written, (unsigned)sizeof(seed));
    return false;
  }
  M5_LOGI("GPS: Warm-start seed saved to %s", kSeedPath);
  return true;
}

bool GpsConnector::injectWarmStartSeedFromSD() {
  if (!SD.exists(kSeedPath)) {
    M5_LOGI("GPS: No warm-start seed found at %s", kSeedPath);
    return false;
  }

  File f = SD.open(kSeedPath, FILE_READ);
  if (!f) {
    M5_LOGE("GPS: Failed to open %s for read", kSeedPath);
    return false;
  }

  // Check file age
  time_t fileTime = f.getLastWrite();
  time_t now = time(nullptr);
  if (now > 0 && fileTime > 0 && (now - fileTime) > kSeedMaxAge) {
    f.close();
    M5_LOGW("GPS: Warm seed too old (%u seconds), skipping injection", (unsigned)(now - fileTime));
    return false;
  }

  WarmSeedBinV1 seed{};
  size_t bytesRead = f.read((uint8_t*)&seed, sizeof(seed));
  f.close();

  if (bytesRead != sizeof(seed)) {
    M5_LOGE("GPS: Invalid seed file size: %u bytes (expected %u)", bytesRead, sizeof(seed));
    return false;
  }

  if (seed.magic != 0x47574D53) { // 'GWMS'
    M5_LOGE("GPS: Invalid seed magic: 0x%08X", seed.magic);
    return false;
  }

  if (seed.version != 1) {
    M5_LOGW("GPS: Unsupported seed version: %u", seed.version);
    return false;
  }

  // Convert back to degrees and meters
  double lat = (double)seed.lat_e7 / 1e7;
  double lon = (double)seed.lon_e7 / 1e7;
  double alt = (double)seed.alt_mm / 1000.0;

  M5_LOGI("GPS: Loaded warm seed from SD: %.6f,%.6f,%.1fm %04u-%02u-%02u %02u:%02u:%02u",
          lat, lon, alt, seed.year, seed.month, seed.day, seed.hour, seed.minute, seed.second);

  // Inject MGA-INI-TIME_UTC
  bool timeOk = sendMgaIniTimeUtc(this, seed.year, seed.month, seed.day, seed.hour, seed.minute, seed.second);
  if (timeOk) {
    M5_LOGI("GPS: MGA-INI-TIME_UTC injected");
  } else {
    M5_LOGW("GPS: MGA-INI-TIME_UTC injection failed");
  }

  // Inject MGA-INI-POS_LLH
  bool posOk = sendMgaIniPosLlh(this, seed.lat_e7, seed.lon_e7, seed.alt_mm);
  if (posOk) {
    M5_LOGI("GPS: MGA-INI-POS_LLH injected");
  } else {
    M5_LOGW("GPS: MGA-INI-POS_LLH injection failed");
  }

  // Minimal delay during boot
  delay(20);

  return timeOk && posOk;
}

void GpsConnector::deactivate() {
  _tried_9600_at = 0;
  _tried_38400_at = 0;
  _tried_115200_at = 0;
  _tried_4800_at = 0;
  _raw_dump_done = false;
  _nmea_mode = false;
  _nmea_len = 0;
  _serial_conn.end();
}

// ---- UBX MGA-DBD dump/restore ----

// Pack and send UBX-MGA-INI-TIME_UTC
static bool sendMgaIniTimeUtc(GpsConnector* gps, uint16_t year, uint8_t month, uint8_t day, 
                              uint8_t hour, uint8_t minute, uint8_t second) {
  // UBX-MGA-INI-TIME_UTC payload (24 bytes)
  uint8_t payload[24] = {0};
  payload[0] = 0x10; // type = TIME_UTC
  payload[1] = 0x00; // version
  // reserved0[2] = 0
  payload[4] = year & 0xFF; payload[5] = (year >> 8) & 0xFF; // year
  payload[6] = month; // month
  payload[7] = day;   // day
  payload[8] = hour;  // hour
  payload[9] = minute; // min
  payload[10] = second; // sec
  // reserved1 = 0
  // ns, tAcc, reserved2 = 0 (we don't have sub-second precision)
  return gps->sendUBXMessage(0x13, 0x40, payload, sizeof(payload));
}

// Pack and send UBX-MGA-INI-POS_LLH
static bool sendMgaIniPosLlh(GpsConnector* gps, int32_t lat_1e7, int32_t lon_1e7, int32_t alt_mm) {
  // UBX-MGA-INI-POS_LLH payload (20 bytes)
  uint8_t payload[20] = {0};
  payload[0] = 0x01; // type = POS_LLH
  payload[1] = 0x00; // version
  // reserved0[2] = 0
  // lat (4 bytes, little-endian, 1e-7 degrees)
  payload[4] = lat_1e7 & 0xFF; payload[5] = (lat_1e7 >> 8) & 0xFF;
  payload[6] = (lat_1e7 >> 16) & 0xFF; payload[7] = (lat_1e7 >> 24) & 0xFF;
  // lon (4 bytes, little-endian, 1e-7 degrees)
  payload[8] = lon_1e7 & 0xFF; payload[9] = (lon_1e7 >> 8) & 0xFF;
  payload[10] = (lon_1e7 >> 16) & 0xFF; payload[11] = (lon_1e7 >> 24) & 0xFF;
  // alt (4 bytes, little-endian, mm)
  payload[12] = alt_mm & 0xFF; payload[13] = (alt_mm >> 8) & 0xFF;
  payload[14] = (alt_mm >> 16) & 0xFF; payload[15] = (alt_mm >> 24) & 0xFF;
  // posAcc = 0 (we set a large default uncertainty)
  uint32_t posAcc = 10000000; // 10km uncertainty in mm
  payload[16] = posAcc & 0xFF; payload[17] = (posAcc >> 8) & 0xFF;
  payload[18] = (posAcc >> 16) & 0xFF; payload[19] = (posAcc >> 24) & 0xFF;
  return gps->sendUBXMessage(0x13, 0x40, payload, sizeof(payload));
}

// Read a single UBX frame from _serial_conn. Returns total bytes read into buf, or 0 on timeout/failure.
static size_t readOneUbxFrame(HardwareSerial& ser, uint8_t* buf, size_t bufsize, uint32_t timeout_ms) {
  const uint32_t start = millis();
  enum { SYNC1, SYNC2, CLASS, ID, LEN1, LEN2, PAYLOAD, CK_A, CK_B } state = SYNC1;
  uint16_t len = 0; size_t idx = 0; uint8_t ck_a = 0, ck_b = 0; size_t payload_read = 0;
  while (millis() - start < timeout_ms) {
    if (ser.available() == 0) { delay(1); continue; }
    uint8_t b = ser.read();
    switch (state) {
      case SYNC1:
        if (b == 0xB5) { if (idx < bufsize) buf[idx++] = b; state = SYNC2; }
        break;
      case SYNC2:
        if (b == 0x62) { if (idx < bufsize) buf[idx++] = b; state = CLASS; }
        else { state = SYNC1; idx = 0; }
        break;
      case CLASS:
        if (idx < bufsize) buf[idx++] = b; ck_a = b; ck_b = ck_a; state = ID; break;
      case ID:
        if (idx < bufsize) buf[idx++] = b; ck_a += b; ck_b += ck_a; state = LEN1; break;
      case LEN1:
        if (idx < bufsize) buf[idx++] = b; ck_a += b; ck_b += ck_a; len = b; state = LEN2; break;
      case LEN2:
        if (idx < bufsize) buf[idx++] = b; ck_a += b; ck_b += ck_a; len |= (uint16_t)b << 8; payload_read = 0; state = (len == 0 ? CK_A : PAYLOAD); break;
      case PAYLOAD:
        if (idx < bufsize) buf[idx++] = b; ck_a += b; ck_b += ck_a; if (++payload_read >= len) state = CK_A; break;
      case CK_A:
        if (idx < bufsize) buf[idx++] = b; if (b != ck_a) { state = SYNC1; idx = 0; } else state = CK_B; break;
      case CK_B:
        if (idx < bufsize) buf[idx++] = b; if (b != ck_b) { state = SYNC1; idx = 0; }
        else { return idx; }
        break;
    }
  }
  return 0; // timeout
}

bool GpsConnector::dumpDatabaseToSD() {
  // Ensure directory exists
  if (!SD.exists(kSeedDir)) {
    if (!SD.mkdir(kSeedDir)) {
      M5_LOGE("GPS: Failed to create %s directory on SD", kSeedDir);
      return false;
    }
  }

  File f = SD.open(kDbdPath, FILE_WRITE);
  if (!f) {
    M5_LOGE("GPS: Failed to open %s for write", kDbdPath);
    return false;
  }

  M5_LOGI("GPS: Requesting GNSS database dump to SD...");
  // Send UBX-MGA-DBD (class 0x13, id 0x80) with zero-length payload to request dump
  if (!sendUBXMessage(0x13, 0x80, nullptr, 0)) {
    M5_LOGE("GPS: Failed to send MGA-DBD request");
    f.close();
    return false;
  }

  // Read frames for a limited time; write only MGA-DBD frames
  const uint32_t overall_start = millis();
  uint32_t last_frame_time = overall_start;
  uint32_t frames = 0, bytes_written = 0;
  uint8_t frame[2048];

  while (millis() - last_frame_time < 1000 && millis() - overall_start < 8000) { // 1s quiet or max 8s
    size_t n = readOneUbxFrame(_serial_conn, frame, sizeof(frame), 250);
    if (n == 0) continue;
    last_frame_time = millis();
    // Check class/id
    if (n >= 6) {
      uint8_t cls = frame[2];
      uint8_t id  = frame[3];
      if (cls == 0x13 && id == 0x80) { // MGA-DBD
        size_t w = f.write(frame, n);
        if (w != n) { M5_LOGE("GPS: SD write error during DBD dump"); break; }
        frames++;
        bytes_written += w;
      }
    }
  }
  f.flush();
  f.close();
  if (frames == 0) {
    M5_LOGW("GPS: No MGA-DBD frames captured");
    return false;
  }
  M5_LOGI("GPS: DBD dump complete: %u frames, %u bytes to %s", frames, bytes_written, kDbdPath);
  return true;
}

// Wait for MGA-ACK-DATA0 after sending a frame; return true if seen
static bool waitForMgaAck(HardwareSerial& ser, uint32_t timeout_ms) {
  uint32_t start = millis();
  uint8_t frame[256];
  while (millis() - start < timeout_ms) {
    size_t n = readOneUbxFrame(ser, frame, sizeof(frame), 50);
    if (n >= 6) {
      uint8_t cls = frame[2];
      uint8_t id  = frame[3];
      if (cls == 0x13 && id == 0x60) { // MGA-ACK-DATA0
        return true;
      }
    }
  }
  return false;
}

// Parse concatenated UBX frames from a File and send them one by one
bool GpsConnector::restoreDatabaseFromSD() {
  if (!SD.exists(kDbdPath)) {
    M5_LOGI("GPS: No database dump found at %s", kDbdPath);
    return false;
  }
  File f = SD.open(kDbdPath, FILE_READ);
  if (!f) {
    M5_LOGE("GPS: Failed to open %s for read", kDbdPath);
    return false;
  }

  // Check file age
  time_t fileTime = f.getLastWrite();
  time_t now = time(nullptr);
  if (now > 0 && fileTime > 0 && (now - fileTime) > kDbdMaxAge) {
    f.close();
    M5_LOGW("GPS: Database dump too old (%u seconds), skipping restore", (unsigned)(now - fileTime));
    return false;
  }

  size_t fileSize = f.size();
  M5_LOGI("GPS: Restoring GNSS database from %s (%u bytes)...", kDbdPath, (unsigned)fileSize);

  if (fileSize == 0) {
    f.close();
    M5_LOGW("GPS: Database file is empty");
    return false;
  }

  size_t frames = 0; size_t bytes = 0; uint8_t b; size_t totalRead = 0;
  enum { FIND_SYNC1, FIND_SYNC2, READ_HEADER, READ_PAYLOAD, READ_CKS } state = FIND_SYNC1;
  uint8_t header[4]; size_t hidx = 0; uint16_t len = 0; uint16_t pleft = 0; uint8_t ck_a = 0, ck_b = 0;
  // Buffer to hold and re-send a full frame
  static const size_t MAXF = 2048;
  uint8_t frame[MAXF]; size_t fidx = 0;

  while (f.available()) {
    totalRead++;
    int bi = f.read();
    if (bi < 0) break;
    b = (uint8_t)bi;
    switch (state) {
      case FIND_SYNC1:
        if (b == 0xB5) { fidx = 0; frame[fidx++] = b; state = FIND_SYNC2; }
        break;
      case FIND_SYNC2:
        if (b == 0x62) { frame[fidx++] = b; state = READ_HEADER; hidx = 0; }
        else { state = FIND_SYNC1; fidx = 0; }
        break;
      case READ_HEADER:
        header[hidx++] = b; frame[fidx++] = b;
        if (hidx == 4) {
          ck_a = header[0]; ck_b = ck_a; // class
          ck_a += header[1]; ck_b += ck_a; // id
          ck_a += header[2]; ck_b += ck_a; // len LSB
          ck_a += header[3]; ck_b += ck_a; // len MSB
          len = (uint16_t)header[2] | ((uint16_t)header[3] << 8);
          pleft = len;
          state = (len == 0) ? READ_CKS : READ_PAYLOAD;
        }
        break;
      case READ_PAYLOAD:
        frame[fidx++] = b;
        ck_a += b; ck_b += ck_a;
        if (--pleft == 0) { state = READ_CKS; }
        break;
      case READ_CKS:
        frame[fidx++] = b;
        if (fidx == 6 + len + 1) { // just read CK_A
          if (b != ck_a) { state = FIND_SYNC1; fidx = 0; break; }
        } else if (fidx == 6 + len + 2) { // just read CK_B
          if (b != ck_b) { state = FIND_SYNC1; fidx = 0; break; }
          // Valid UBX frame complete; send and wait for ACK if it's DBD
          uint8_t cls = header[0];
          uint8_t id  = header[1];
          if (cls == 0x13 && id == 0x80) {
            _serial_conn.write(frame, fidx);
            _serial_conn.flush();
            bytes += fidx; frames++;
            // Minimal pacing - don't wait for ACKs during boot
            delay(1);
          }
          // Reset for next frame
          state = FIND_SYNC1; fidx = 0;
        }
        break;
    }
  }
  f.close();
  M5_LOGI("GPS: Database restore sent: %u frames, %u bytes (read %u/%u bytes from file)", 
          (unsigned)frames, (unsigned)bytes, (unsigned)totalRead, (unsigned)fileSize);
  return frames > 0;
}

/**
 * Set the GPS dynamic platform model
 * @param model The dynamic model to set (e.g., DYNMODEL_PORT, DYNMODEL_AIR4)
 * @param saveToFlash If true, saves the setting to flash memory so it persists after power cycles
 * @return true if successful, false otherwise
 */
bool GpsConnector::setDynamicModel(UbxDynamicModel model, bool saveToFlash) {
  bool success = false;
  M5_LOGD("GNSS: Setting dynamic model to %d (saveToFlash: %d)", model, saveToFlash);

  if (data.protocolVersionHigh > 0 && data.protocolVersionHigh < 18) {
    // u-blox 7 (e.g. UBX-G7020-KT): use UBX-CFG-NAV5 (0x06/0x24)
    // CFG-VALSET is not available on protocol versions below 18
    uint8_t nav5Payload[36] = {0};
    nav5Payload[0] = 0x01; // mask LSB: apply dynModel only
    nav5Payload[1] = 0x00; // mask MSB
    nav5Payload[2] = (uint8_t)model; // dynModel
    nav5Payload[3] = 0x03; // fixMode: auto 2D/3D
    if (sendUBXMessage(UBX_CLASS_CFG, UBX_ID_CFG_NAV5, nav5Payload, sizeof(nav5Payload))) {
      delay(100);
      _current_model = model;
      success = true;
    }
  } else {
    // u-blox 8+ (M8/M9/M10): use UBX-CFG-VALSET (0x06/0x8A)
    uint8_t msgPayload[12] = {0};
    msgPayload[0] = 0x00; // version 0
    msgPayload[1] = saveToFlash ? UBX_CFG_LAYER_ALL : UBX_CFG_LAYER_RAM;
    // Key: CFG-NAVSPG-DYNMODEL (0x20110021)
    msgPayload[4] = 0x21;
    msgPayload[5] = 0x00;
    msgPayload[6] = 0x11;
    msgPayload[7] = 0x20;
    msgPayload[8] = (uint8_t)model;
    if (sendUBXMessage(UBX_CLASS_CFG, UBX_CFG_VALSET, msgPayload, sizeof(msgPayload))) {
      delay(100);
      _current_model = model;
      success = true;
    }
  }

  if (success) {
    const char* modelName = "UNKNOWN";
    switch(model) {
      case DYNMODEL_PORT: modelName = "PORTABLE"; break;
      case DYNMODEL_STATIONARY: modelName = "STATIONARY"; break;
      case DYNMODEL_PEDESTRIAN: modelName = "PEDESTRIAN"; break;
      case DYNMODEL_AUTOMOTIVE: modelName = "AUTOMOTIVE"; break;
      case DYNMODEL_SEA: modelName = "SEA"; break;
      case DYNMODEL_AIRBORNE_1G: modelName = "AIRBORNE 1G"; break;
      case DYNMODEL_AIRBORNE_2G: modelName = "AIRBORNE 2G"; break;
      case DYNMODEL_AIRBORNE_4G: modelName = "AIRBORNE 4G"; break;
      case DYNMODEL_WRIST: modelName = "WRIST"; break;
    }
    M5_LOGD("GNSS: Set to %s mode (saveToFlash: %d)", modelName, saveToFlash);
  } else {
    M5_LOGD("GNSS: Failed to send dynamic model change command");
  }

  return success;
}

/**
 * Set the GPS dynamic platform model (RAM only version)
 * @param model The dynamic model to set (e.g., DYNMODEL_PORT, DYNMODEL_AIR4)
 * @return true if successful, false otherwise
 */
bool GpsConnector::setDynamicModel(UbxDynamicModel model) {
  // Call the extended version with saveToFlash = false
  return setDynamicModel(model, false);
}

/**
 * Get the current GPS dynamic platform model
 * @return The current dynamic model
 */
UbxDynamicModel GpsConnector::getDynamicModel() {
  // Return the stored current model
  return _current_model;
}

/**
 * Read the current dynamic model directly from the GPS module
 * @return true if successful, false otherwise
 */
bool GpsConnector::readDynamicModelFromGPS() {
  bool success = false;

  if (data.protocolVersionHigh > 0 && data.protocolVersionHigh < 18) {
    // u-blox 7: poll UBX-CFG-NAV5 (no payload = poll request)
    success = sendUBXMessage(UBX_CLASS_CFG, UBX_ID_CFG_NAV5, nullptr, 0);
  } else {
    // u-blox 8+ (M8/M9/M10): poll UBX-CFG-VALGET
    uint8_t msgPayload[8] = {0};
    msgPayload[1] = UBX_CFG_LAYER_RAM;
    msgPayload[4] = 0x21; // Key: CFG-NAVSPG-DYNMODEL LSB
    msgPayload[5] = 0x00;
    msgPayload[6] = 0x11;
    msgPayload[7] = 0x20; // MSB
    success = sendUBXMessage(UBX_CLASS_CFG, UBX_CFG_VALGET, msgPayload, sizeof(msgPayload));
  }

  if (success) {
    delay(100);
    M5_LOGD("GNSS: Current dynamic model (stored): %d", _current_model);
  } else {
    M5_LOGD("GNSS: Failed to send dynamic model query command");
  }

  return success;
}


int8_t GpsConnector::produce_data() {
  if (_nmea_mode) {
    return produceDataNmea();
  }
  auto ret_status = e_worker_idle;

  // getPVT returns true if there is a fresh navigation solution available.
  // "LLH" is longitude, latitude, height.
  // getPVT() returns UTC date and time.
  // Do not use GNSS time, see u-blox spec section 9.
  if (_gnss.getNAVPVT()) {
    // M5_LOGD("[%d] _gnss.getPVT() is true.", millis());

    data.satsInView = _gnss.getNumSV(); // Satellites In View (numSV)

    if (_gnss.getFixType() == 2 || _gnss.getFixType() == 3) {
      // M5_LOGD("[%d] fix type is 2D or 3D.", millis());
      data.pdop = _gnss.getPDOP() * 1e-2; // Position Dilution of Precision
      // Bypass distance gate on the very first fix; otherwise haversine() to
      // (0,0) is huge and the first valid sample would be discarded.
      const bool first_fix = (_last_latitude == 0.0 && _last_longitude == 0.0);
      _last_latitude = data.latitude;
      _last_longitude = data.longitude;
      data.latitude = _gnss.getLatitude() * 1e-7;
      data.longitude = _gnss.getLongitude() * 1e-7;
      data.heading_degree = _gnss.getHeading() * 1e-5;
      if (data.heading_degree >= NNW || data.heading_degree < NNE) {
        data.heading = GnssData::NORTH;
      }
      if (data.heading_degree >= NNE || data.heading_degree < ENE) {
        data.heading = GnssData::NORTHEAST;
      }
      if (data.heading_degree >= ENE || data.heading_degree < ESE) {
        data.heading = GnssData::EAST;
      }
      if (data.heading_degree >= ESE || data.heading_degree < SSE) {
        data.heading = GnssData::SOUTHEAST;
      }
      if (data.heading_degree >= SSE || data.heading_degree < SSW) {
        data.heading = GnssData::SOUTH;
      }
      if (data.heading_degree >= SSW || data.heading_degree < WSW) {
        data.heading = GnssData::SOUTHWEST;
      }
      if (data.heading_degree >= WSW || data.heading_degree < WNW) {
        data.heading = GnssData::WEST;
      }
      if (data.heading_degree >= WNW || data.heading_degree < NNW) {
        data.heading = GnssData::NORTHWEST;
      }
      data.altitudeMSL = _gnss.getFixType() == 3 ? _gnss.getAltitudeMSL() * 1e-3 : 0; // Above MSL (not ellipsoid)
      const auto distance_step = haversine_km(data.latitude, data.longitude, _last_latitude, _last_longitude);


      // Retrieve extra confidence indicators from NavPvt
      data.hAcc = _gnss.getHorizontalAccEst();  // mm Horizontal accuracy estimate for Long/Lat
      data.vAcc = _gnss.getVerticalAccEst();  // mm Vertical accuracy estimate for Long/Lat
      data.velN = _gnss.getVelN();  // mm/s NED north velocity
      data.velE = _gnss.getVelE();  // mm/s NED east velocity
      data.velD = _gnss.getVelD();  // mm/s NED down velocity
      data.gSpeed = _gnss.getGroundSpeed();  // Ground Speed (2-D)
      data.headMot = _gnss.getHeading();  // Heading of motion (2-D)
      data.sAcc = _gnss.getSpeedAccEst();
      data.headAcc = _gnss.getHeadingAccEst();

      data.location_valid = _gnss.getGnssFixOk() && (first_fix || distance_step < 0.5); // Airplanes fly at 255m per second, 500 meter check is good enough
      location_timer.restart();
      ret_status = e_worker_data_read;
      // DEBUG: Compare PDOP from NAV-PVT and HDOP from NAV-DOP
      // M5_LOGD("[%d] GnssFixOk (type = %d)."
      //               "  SATS: %d; PDOP: %d; HDOP: %d\n",
      //               millis(), _gnss.getFixType(),
      //               data.satsInView, _gnss.getPDOP(), data.hdop);
    }

    if (_gnss.getDateValid()) {
//      M5_LOGD("[%lu] _gnss.getDateValid() is true.", millis());
      data.year = _gnss.getYear();
      data.month = _gnss.getMonth();
      data.day = _gnss.getDay();
      data.date_valid = true;
      date_timer.restart();
      ret_status = e_worker_data_read;
    }

    if (_gnss.getTimeValid()) {
//      M5_LOGD("[%lu] _gnss.getTimeValid() is true.", millis());
      data.hour = _gnss.getHour();
      data.minute = _gnss.getMinute();
      data.second = _gnss.getSecond();
      data.time_valid = true;
      time_timer.restart();
      ret_status = e_worker_data_read;
    }
  }

  // Check expiry
  if (location_timer.isExpired()) {
    data.location_valid = false;
  }
  if (time_timer.isExpired()) {
    data.time_valid = false;
    data.hour = GPS_INVALID_HOUR;
    data.minute = GPS_INVALID_MINUTE;
    data.second = GPS_INVALID_SECOND;
  }
  if (date_timer.isExpired()) {
    data.date_valid = false;
    data.year = GPS_INVALID_YEAR;
    data.month = GPS_INVALID_MONTH;
    data.day = GPS_INVALID_DAY;
  }

  ApiDataCache::instance().update(data);
  return ret_status;
}

/**
 * Send a raw UBX message to the GPS module
 * @param msgClass UBX message class
 * @param msgID UBX message ID
 * @param payload Payload data
 * @param payloadSize Size of the payload
 * @return true if successful, false otherwise
 */
bool GpsConnector::sendUBXMessage(uint8_t msgClass, uint8_t msgID, const uint8_t* payload, size_t payloadSize) {
  // UBX message structure:
  // Sync Char 1: 0xB5
  // Sync Char 2: 0x62
  // Class: 1 byte
  // ID: 1 byte
  // Length: 2 bytes (little endian)
  // Payload: variable length
  // Checksum: 2 bytes (CK_A, CK_B)
  
  // Write sync chars
  _serial_conn.write(UBX_SYNC_CHAR_1);
  _serial_conn.write(UBX_SYNC_CHAR_2);
  
  // Write class and ID
  _serial_conn.write(msgClass);
  _serial_conn.write(msgID);
  
  // Write length (little endian)
  _serial_conn.write(payloadSize & 0xFF);
  _serial_conn.write((payloadSize >> 8) & 0xFF);
  
  // Write payload
  if (payload != nullptr && payloadSize > 0) {
    _serial_conn.write(payload, payloadSize);
  }

  // Calculate checksum
  uint8_t ck_a = 0, ck_b = 0;

  // Add class, ID, and length to checksum
  ck_a += msgClass;
  ck_b += ck_a;

  ck_a += msgID;
  ck_b += ck_a;

  ck_a += payloadSize & 0xFF;
  ck_b += ck_a;

  ck_a += (payloadSize >> 8) & 0xFF;
  ck_b += ck_a;

  // Add payload to checksum
  if (payload != nullptr) {
    for (size_t i = 0; i < payloadSize; i++) {
      ck_a += payload[i];
      ck_b += ck_a;
    }
  }
  
  // Write checksum
  _serial_conn.write(ck_a);
  _serial_conn.write(ck_b);
  
  // Flush the serial buffer
  _serial_conn.flush();
  
  // Wait a bit for the message to be processed
  delay(5);
  
  return true;
}

/**
 * Calculate UBX message checksum
 * @param data Data to calculate checksum for
 * @param len Length of data
 * @param cka Pointer to store CK_A
 * @param ckb Pointer to store CK_B
 */
void GpsConnector::calculateChecksum(const uint8_t* data, size_t len, uint8_t* cka, uint8_t* ckb) {
  *cka = 0;
  *ckb = 0;
  
  for (size_t i = 0; i < len; i++) {
    *cka += data[i];
    *ckb += *cka;
  }
}

/**
 * Ask the GNSS to enter Backup mode (save state) using UBX-RXM-PMREQ.
 * This dramatically reduces TTFF on the next boot.
 */
bool GpsConnector::requestBackup() {
  // UBX-RXM-PMREQ (0x02 0x41) payload is 8 bytes:
  // 0-1: version / reserved (set to 0)
  // 2-3: duration (little-endian) – 0 means indefinite
  // 4-7: flags – bit0 = backup, everything else 0
  const uint8_t payload[8] = { 0x00, 0x00, // version / reserved
                               0x00, 0x00, // duration LSB/MSB (0 = indefinitely)
                               0x01, 0x00, 0x00, 0x00 }; // flags (backup)
  return sendUBXMessage(0x02 /*RXM*/, 0x41 /*PMREQ*/, payload, sizeof(payload));
}

bool GpsConnector::backupGpsMemoryToNVS() {
  M5_LOGI("GPS: Starting memory backup to internal storage");
  
  // Request almanac data using UBX-MGA-GPS-ALM
  // This is a simplified implementation - full implementation would need
  // to handle multiple UBX-MGA message types and responses
  
  // For now, we'll use the existing backup functionality which saves
  // the GPS state to its internal backup memory
  bool success = requestBackup();
  
  if (success) {
    M5_LOGI("GPS: Memory backup completed - GPS internal state saved");
  } else {
    M5_LOGE("GPS: Memory backup failed");
  }
  
  return success;
}

bool GpsConnector::restoreGpsMemoryFromNVS() {
  M5_LOGI("GPS: Checking for saved memory data in internal storage");
  M5_LOGI("GPS: Memory restore relies on U-blox internal backup memory");
  return true;
}

// ---------------------------------------------------------------------------
// NMEA fallback mode — used when GPS TX is connected but GPS RX is not,
// so only one-way communication (receive NMEA) is possible.
// ---------------------------------------------------------------------------

double GpsConnector::nmeaCoordToDecimal(const char* coord, char direction) {
  double val = atof(coord);
  int deg = (int)(val / 100.0);
  double min = val - deg * 100.0;
  double result = deg + min / 60.0;
  if (direction == 'S' || direction == 'W') result = -result;
  return result;
}

bool GpsConnector::parseNmeaSentence(const char* sentence) {
  // Validate checksum (XOR of bytes between '$' and '*')
  const char* star = strchr(sentence, '*');
  if (star != nullptr && strlen(star) >= 3) {
    uint8_t calc = 0;
    for (const char* p = sentence + 1; p < star; p++) calc ^= (uint8_t)*p;
    uint8_t recv = (uint8_t)strtol(star + 1, nullptr, 16);
    if (calc != recv) return false;
  }

  bool isRmc = (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0);
  bool isGga = (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0);
  bool isGsv = (strncmp(sentence, "$GPGSV", 6) == 0 || strncmp(sentence, "$GLGSV", 6) == 0 || strncmp(sentence, "$GNGSV", 6) == 0);
  bool isGsa = (strncmp(sentence, "$GPGSA", 6) == 0 || strncmp(sentence, "$GNGSA", 6) == 0);
  if (!isRmc && !isGga && !isGsv && !isGsa) return false;

  // Determine constellation type char from sentence prefix for GSV
  char gnssIdType = 'G'; // default GPS
  if (strncmp(sentence, "$GL", 3) == 0) gnssIdType = 'R'; // GLONASS
  else if (strncmp(sentence, "$GA", 3) == 0) gnssIdType = 'E'; // Galileo
  else if (strncmp(sentence, "$GB", 3) == 0) gnssIdType = 'B'; // BeiDou

  // Copy and strip checksum for field splitting
  char buf[128];
  strncpy(buf, sentence, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';
  char* s = strchr(buf, '*');
  if (s) *s = '\0';

  // Split on commas (GPGSV needs up to 20 fields, GPGSA up to 18)
  char* fields[20] = {nullptr};
  int nf = 0;
  char* p = buf;
  while (nf < 20) {
    fields[nf++] = p;
    p = strchr(p, ',');
    if (!p) break;
    *p++ = '\0';
  }

  if (isRmc && nf >= 10) {
    // $G?RMC,HHMMSS.ss,A,DDMM.mmm,N,DDDMM.mmm,E,speed,course,DDMMYY,...
    const char* tstr = fields[1];
    const char* stat = fields[2];
    const char* lat  = fields[3];
    const char* ns   = fields[4];
    const char* lon  = fields[5];
    const char* ew   = fields[6];
    const char* spd  = fields[7];
    const char* crs  = fields[8];
    const char* dstr = fields[9];

    if (strlen(tstr) >= 6) {
      char tmp[3] = {tstr[0], tstr[1], '\0'}; data.hour   = atoi(tmp);
      tmp[0] = tstr[2]; tmp[1] = tstr[3];     data.minute = atoi(tmp);
      tmp[0] = tstr[4]; tmp[1] = tstr[5];     data.second = atoi(tmp);
      data.time_valid = true;
      time_timer.restart();
    }
    if (strlen(dstr) >= 6) {
      char tmp[3] = {dstr[0], dstr[1], '\0'}; data.day   = atoi(tmp);
      tmp[0] = dstr[2]; tmp[1] = dstr[3];     data.month = atoi(tmp);
      tmp[0] = dstr[4]; tmp[1] = dstr[5];     data.year  = 2000 + atoi(tmp);
      data.date_valid = true;
      date_timer.restart();
    }
    if (stat[0] == 'A' && strlen(lat) > 0 && strlen(lon) > 0) {
      double new_lat = nmeaCoordToDecimal(lat, ns[0]);
      double new_lon = nmeaCoordToDecimal(lon, ew[0]);
      // Skip distance sanity check on first fix (last position is 0,0)
      bool first_fix = (_last_latitude == 0.0 && _last_longitude == 0.0);
      const auto dist = haversine_km(new_lat, new_lon, _last_latitude, _last_longitude);
      _last_latitude  = data.latitude;
      _last_longitude = data.longitude;
      data.latitude  = new_lat;
      data.longitude = new_lon;
      data.gSpeed    = (int32_t)(atof(spd) * 514.444); // knots → mm/s
      data.heading_degree = strlen(crs) > 0 ? atof(crs) : 0.0;
      data.location_valid = first_fix || (dist < 0.5);
      location_timer.restart();
      return true;
    }
  }

  if (isGga && nf >= 10) {
    // $G?GGA,time,lat,N,lon,E,quality,nsats,hdop,alt,M,...
    const char* qual  = fields[6];
    const char* nsats = fields[7];
    const char* hdop  = fields[8];
    const char* alt   = fields[9];
    if (qual[0] != '0' && strlen(nsats) > 0) {
      data.satsInView = atoi(nsats);
      data.numSV      = data.satsInView;
      data.pdop       = strlen(hdop) > 0 ? atof(hdop) : 0.0;
      data.altitudeMSL = strlen(alt) > 0 ? atof(alt) : 0.0;
      return true;
    }
  }

  if (isGsv && nf >= 4) {
    // $G?GSV,numMsgs,msgNum,numSV,sv1,elev1,azim1,cno1[,...]*cs
    int totalMsgs = atoi(fields[1]);
    int msgNum = atoi(fields[2]);
    if (msgNum == 1) {
      // First message: reset accumulator
      data.nmea_sat_count = 0;
      data.numSV = atoi(fields[3]); // total sats in view (not used in fix)
      // satsInView is set from GPGSA (used-in-fix count) — don't overwrite here
    }
    // Each message carries up to 4 sats starting at field index 4
    for (int i = 0; i < 4 && data.nmea_sat_count < GnssData::NMEA_MAX_SATS; i++) {
      int base = 4 + i * 4;
      if (base + 3 >= nf || !fields[base] || strlen(fields[base]) == 0) break;
      GnssData::NmeaSatEntry& sat = data.nmea_sats[data.nmea_sat_count++];
      sat.svId       = (uint8_t)atoi(fields[base]);
      sat.elev       = (int8_t)atoi(fields[base + 1]);
      sat.azim       = (int16_t)atoi(fields[base + 2]);
      sat.cno        = (fields[base + 3] && strlen(fields[base + 3]) > 0)
                       ? (uint8_t)atoi(fields[base + 3]) : 0;
      sat.gnssIdType = gnssIdType;
      // Mark as used if svId is in the GPGSA used list
      sat.svUsed = false;
      for (uint8_t j = 0; j < _nmea_used_count; j++) {
        if (_nmea_used_svids[j] == sat.svId) { sat.svUsed = true; break; }
      }
    }
    if (msgNum == totalMsgs) {
      data.nmea_gsv_cycle++; // signal a complete GPGSV cycle to NavsatCollector
    }
    return true;
  }

  if (isGsa && nf >= 3) {
    // $G?GSA,mode,fix,sv1..sv12,pdop,hdop,vdop
    // fields[3..14] are SV IDs used (may be blank)
    _nmea_used_count = 0;
    for (int i = 3; i <= 14 && i < nf && _nmea_used_count < 12; i++) {
      if (fields[i] && strlen(fields[i]) > 0) {
        _nmea_used_svids[_nmea_used_count++] = (uint8_t)atoi(fields[i]);
      }
    }
    // In NMEA mode satsInView reflects sats used in fix (consistent with UBX NAV-PVT numSV)
    data.satsInView = _nmea_used_count;
    // Update svUsed flag for already-collected satellite entries
    for (uint8_t i = 0; i < data.nmea_sat_count; i++) {
      data.nmea_sats[i].svUsed = false;
      for (uint8_t j = 0; j < _nmea_used_count; j++) {
        if (_nmea_used_svids[j] == data.nmea_sats[i].svId) {
          data.nmea_sats[i].svUsed = true;
          break;
        }
      }
    }
    return true;
  }

  return false;
}

int8_t GpsConnector::produceDataNmea() {
  auto ret_status = e_worker_idle;
  static bool first_fix_logged = false;

  while (_serial_conn.available()) {
    char c = (char)_serial_conn.read();
    if (c == '$') {
      _nmea_len = 0;
      _nmea_buf[_nmea_len++] = c;
    } else if (c == '\n' || c == '\r') {
      if (_nmea_len > 6) {
        _nmea_buf[_nmea_len] = '\0';
        if (parseNmeaSentence(_nmea_buf)) {
          ret_status = e_worker_data_read;
          if (!first_fix_logged && data.location_valid) {
            first_fix_logged = true;
            M5_LOGI("GNSS: NMEA fix — lat=%.5f lon=%.5f alt=%.1fm sats=%d",
                    data.latitude, data.longitude, data.altitudeMSL, data.satsInView);
          }
        }
      }
      _nmea_len = 0;
    } else if (_nmea_len > 0 && _nmea_len < (uint8_t)(sizeof(_nmea_buf) - 1)) {
      _nmea_buf[_nmea_len++] = c;
    }
  }

  if (location_timer.isExpired()) data.location_valid = false;
  if (time_timer.isExpired()) {
    data.time_valid = false;
    data.hour = GPS_INVALID_HOUR;
    data.minute = GPS_INVALID_MINUTE;
    data.second = GPS_INVALID_SECOND;
  }
  if (date_timer.isExpired()) {
    data.date_valid = false;
    data.year  = GPS_INVALID_YEAR;
    data.month = GPS_INVALID_MONTH;
    data.day   = GPS_INVALID_DAY;
  }

  ApiDataCache::instance().update(data);
  return ret_status;
}
