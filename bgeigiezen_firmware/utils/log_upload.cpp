#include "log_upload.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "user_config.h"
#include "utils/sd_wrapper.h"
#include "utils/wifi_connection.h"

namespace {
class ChunkedHTTPClient : public HTTPClient {
 public:
  bool sendPOSTHeaders() {
    if (!connect()) {
      return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
    }
    return sendHeader("POST");
  }

  int handleResponse() {
    return handleHeaderResponse();
  }
};
}  // namespace

int upload_log_file(const char* path, const char* file_name, const LocalStorage& config, uint32_t* out_import_id) {
  File log_file = SDInterface::i().get_file(path);
  if (!log_file) {
    M5_LOGD("Failed to open log file!");
    return 0;
  }

  char url[160];
  snprintf(url, sizeof(url), "%s?api_key=%s", API_LOGFILE_ENDPOINT, config.get_api_key());

  ChunkedHTTPClient http;
  WiFiClientSecure client;
  // TODO: pin Safecast CA bundle. Using setInsecure() until cert plumbing lands.
  client.setInsecure();

  if (!http.begin(client, url)) {
    M5_LOGD("Unable to begin URL connection");
    http.end();
    log_file.close();
    return 0;
  }

  char boundary[32];
  sprintf(boundary, "----WebKitFormBoundaryZen%d", config.get_device_id());
  char content_type_header[150];
  sprintf(content_type_header, "multipart/form-data; boundary=%s", boundary);

  http.setUserAgent(HEADER_API_USER_AGENT);
  http.addHeader("Content-Type", content_type_header);
  http.addHeader("Transfer-Encoding", "chunked");
  http.addHeader("Accept", "application/json");

  char chunk_header[16];
  static constexpr size_t CHUNK_BUF_SZ = 1024;
  char chunk_buffer[CHUNK_BUF_SZ];
  int status_code = 0;

  if (http.sendPOSTHeaders()) {
    sprintf(chunk_buffer,
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"bgeigie_import[description]\"\r\n"
            "\r\n"
            "Uploaded from Zen\r\n"
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"bgeigie_import[source]\"; filename=\"%s\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n",
            boundary, boundary, file_name);
    sprintf(chunk_header, "%X\r\n", strlen(chunk_buffer));
    client.print(chunk_header);
    client.print(chunk_buffer);
    client.print("\r\n");

    while (log_file.available()) {
      int n = log_file.read((uint8_t*)chunk_buffer, CHUNK_BUF_SZ);
      if (n <= 0) break;
      sprintf(chunk_header, "%X\r\n", n);
      client.write((const uint8_t*)chunk_header, strlen(chunk_header));
      client.write((const uint8_t*)chunk_buffer, n);
      client.write((const uint8_t*)"\r\n", 2);
      yield();
    }

    sprintf(chunk_buffer, "\r\n--%s--\r\n", boundary);
    sprintf(chunk_header, "%X\r\n", strlen(chunk_buffer));
    client.print(chunk_header);
    client.print(chunk_buffer);
    client.print("\r\n");
    client.print("0\r\n\r\n");

    status_code = http.handleResponse();
    String response = http.getString();
    M5_LOGD("POST complete, (code %d) response:\n%s", status_code, response.c_str());

    if (status_code >= 200 && status_code < 300 && out_import_id) {
      int id_idx = response.indexOf("\"id\":");
      if (id_idx >= 0) {
        *out_import_id = (uint32_t) strtoul(response.c_str() + id_idx + 5, nullptr, 10);
      }
    }
    WiFiWrapper_i.update_active();
  } else {
    M5_LOGD("Failed to initiate request");
  }

  http.end();
  log_file.close();
  return status_code;
}

// Simplemap login: POST {email, api_key} -> session cookie, returned as "name=value" (empty on failure)
static bool simplemap_login(const LocalStorage& config, char* cookie, size_t cookie_size) {
  WiFiClientSecure client;
  client.setInsecure();  // TODO: pin CA, same as API upload
  HTTPClient http;
  const char* headers[] = {"Set-Cookie"};
  http.collectHeaders(headers, 1);
  if (!http.begin(client, SIMPLEMAP_LOGIN_ENDPOINT)) {
    return false;
  }
  http.setUserAgent(HEADER_API_USER_AGENT);
  http.addHeader("Content-Type", "application/json");
  char body[200];
  snprintf(body, sizeof(body), "{\"email\":\"%s\",\"api_key\":\"%s\"}", config.get_simplemap_email(), config.get_api_key());
  int code = http.POST(body);
  bool ok = false;
  if (code == 200 && http.hasHeader("Set-Cookie")) {
    String c = http.header("Set-Cookie");
    int semi = c.indexOf(';');
    if (semi > 0) c = c.substring(0, semi);
    if (c.length() > 0 && c.length() < cookie_size) {
      strcpy(cookie, c.c_str());
      ok = true;
    }
  } else {
    M5_LOGD("Simplemap login failed (code %d)", code);
  }
  http.end();
  return ok;
}

int upload_log_file_simplemap(const char* path, const char* file_name, const LocalStorage& config) {
  if (!strlen(config.get_simplemap_email()) || !strlen(config.get_api_key())) {
    return 0;
  }
  char cookie[200];
  if (!simplemap_login(config, cookie, sizeof(cookie))) {
    return 401;
  }
  File log_file = SDInterface::i().get_file(path);
  if (!log_file) {
    return 0;
  }

  ChunkedHTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // TODO: pin CA, same as API upload

  if (!http.begin(client, SIMPLEMAP_UPLOAD_ENDPOINT)) {
    http.end();
    log_file.close();
    return 0;
  }

  char boundary[32];
  sprintf(boundary, "----WebKitFormBoundaryZen%d", config.get_device_id());
  char content_type_header[150];
  sprintf(content_type_header, "multipart/form-data; boundary=%s", boundary);

  http.setUserAgent(HEADER_API_USER_AGENT);
  http.addHeader("Content-Type", content_type_header);
  http.addHeader("Transfer-Encoding", "chunked");
  http.addHeader("Cookie", cookie);
  http.addHeader("Accept", "application/json");

  char chunk_header[16];
  static constexpr size_t CHUNK_BUF_SZ = 1024;
  char chunk_buffer[CHUNK_BUF_SZ];
  int status_code = 0;

  if (http.sendPOSTHeaders()) {
    sprintf(chunk_buffer,
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"files[]\"; filename=\"%s\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n",
            boundary, file_name);
    sprintf(chunk_header, "%X\r\n", strlen(chunk_buffer));
    client.print(chunk_header);
    client.print(chunk_buffer);
    client.print("\r\n");

    while (log_file.available()) {
      int n = log_file.read((uint8_t*)chunk_buffer, CHUNK_BUF_SZ);
      if (n <= 0) break;
      sprintf(chunk_header, "%X\r\n", n);
      client.write((const uint8_t*)chunk_header, strlen(chunk_header));
      client.write((const uint8_t*)chunk_buffer, n);
      client.write((const uint8_t*)"\r\n", 2);
      yield();
    }

    sprintf(chunk_buffer, "\r\n--%s--\r\n", boundary);
    sprintf(chunk_header, "%X\r\n", strlen(chunk_buffer));
    client.print(chunk_header);
    client.print(chunk_buffer);
    client.print("\r\n");
    client.print("0\r\n\r\n");

    status_code = http.handleResponse();
    String response = http.getString();
    M5_LOGD("Simplemap POST complete, (code %d) response:\n%s", status_code, response.c_str());
    WiFiWrapper_i.update_active();
  }

  http.end();
  log_file.close();
  return status_code;
}
