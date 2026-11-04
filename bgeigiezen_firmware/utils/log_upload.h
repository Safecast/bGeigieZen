#ifndef BGEIGIEZEN_LOG_UPLOAD_H_
#define BGEIGIEZEN_LOG_UPLOAD_H_

#include <Arduino.h>
#include "workers/local_storage.h"

/**
 * Upload a log file from the SD card to the Safecast API (bgeigie_imports), which
 * also makes it available on the map. Blocking.
 * @return HTTP status code, or 0 if the request could not be made
 */
int upload_log_file(const char* path, const char* file_name, const LocalStorage& config, uint32_t* out_import_id = nullptr);

/**
 * Upload a log file to simplemap.safecast.org (/upload, multipart files[], session login with the simplemap email
 * and the same API key as api.safecast.org). Blocking.
 * @return HTTP status code, or 0 if the request could not be made
 */
int upload_log_file_simplemap(const char* path, const char* file_name, const LocalStorage& config, char* out_track_id = nullptr, size_t out_size = 0);

#endif //BGEIGIEZEN_LOG_UPLOAD_H_
