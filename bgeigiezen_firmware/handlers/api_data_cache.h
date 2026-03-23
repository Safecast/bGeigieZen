#ifndef BGEIGIEZEN_API_DATA_CACHE_H_
#define BGEIGIEZEN_API_DATA_CACHE_H_

#include "Handler.hpp"

#define API_DATA_CACHE_SIZE 600

/**
 * Singleton cache of the latest sensor readings, used by the HTTP
 * /api/v1/status endpoint.  Workers call update() each time they produce
 * new data; the web server reads it on demand.
 */
class ApiDataCache : public Handler {
 public:
  static ApiDataCache& instance();
  const char* get_latest_json() const;

 protected:
  int8_t handle_produced_work(const worker_map_t& workers) final;

 private:
  ApiDataCache() = default;

  char _api_data_cache[API_DATA_CACHE_SIZE] = "";
};

#endif // BGEIGIEZEN_API_DATA_CACHE_H_
