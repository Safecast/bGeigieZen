#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SD.h>

#include "log_viewer.h"
#include "identifiers.h"
#include "menu_window.h"
#include "user_config.h"
#include "utils/sd_wrapper.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"
#include "utils/wifi_connection.h"


LogViewerScreen LogViewerScreen_i;


class ChunkedHTTPClient : public HTTPClient {
 public:
  bool sendPOSTHeaders() {

    // connect to server
    if(!connect()) {
      return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
    }

    return sendHeader("POST");
  }

  int handleResponse() {
    return handleHeaderResponse();
  }
};



LogViewerScreen::LogViewerScreen() : BaseScreen("Log view", true),
                                     _current_view(e_log_main_view),
                                     _detail_log_file_path(""),
                                     _detail_log_file_name(""),
                                     _detail_log_timestamp{},
                                     _detail_upload_timestamp{},
                                     _detail_log_upload_id(0),
                                     _file_count(0),
                                     _selected_index(0),
                                     _main_selected_index(0),
                                     _upload_status(e_upload_idle),
                                     _upload_http_code(0) {
  required_wifi = true;
  required_sd = true;
  _file_list[0][0] = '\0';
}

const char* LogViewerScreen::current_dir() const {
  switch (_current_view) {
    case e_log_drive_view:   return DRIVE_LOG_DIRECTORY;
    case e_log_survey_view:  return SURVEY_LOG_DIRECTORY;
    case e_log_journal_view: return JOURNAL_LOG_DIRECTORY;
    case e_log_flight_view:  return FLIGHT_LOG_DIRECTORY;
    default:                 return nullptr;
  }
}

void LogViewerScreen::load_log_list(const char* dir) {
  _file_count = 0;
  _selected_index = 0;
  if (!dir) return;

  File root = SD.open(dir, FILE_READ);
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    return;
  }

  // Scan the whole directory and keep only the LOG_LIST_MAX newest entries.
  // Reading just the first LOG_LIST_MAX in directory order would drop the
  // most recent files when the folder has more than LOG_LIST_MAX logs.
  // Names are dated (YYYY-MM-DD_HHMM.log) so descending strcmp = newest first.
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    if (entry.isDirectory()) {
      entry.close();
      continue;
    }
    const char* name = entry.name();
    const char* slash = strrchr(name, '/');
    const char* base = slash ? slash + 1 : name;
    const char* dot = strrchr(base, '.');
    if (!dot || strcmp(dot, ".log") != 0 || strcmp(base, "latest.log") == 0) {
      entry.close();
      continue;
    }

    // Insertion sort into _file_list, descending. Find the slot, then shift
    // older entries right; the oldest falls off when the list is full.
    int pos = 0;
    while (pos < _file_count && strcmp(_file_list[pos], base) > 0) {
      pos++;
    }
    if (pos < LOG_LIST_MAX) {
      int last = (_file_count < LOG_LIST_MAX) ? _file_count : LOG_LIST_MAX - 1;
      for (int j = last; j > pos; j--) {
        strncpy(_file_list[j], _file_list[j - 1], LOG_NAME_MAX);
      }
      strncpy(_file_list[pos], base, LOG_NAME_MAX - 1);
      _file_list[pos][LOG_NAME_MAX - 1] = '\0';
      if (_file_count < LOG_LIST_MAX) {
        _file_count++;
      }
    }
    entry.close();
  }
  root.close();
}

BaseScreen* LogViewerScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);


  if (strlen(_detail_log_file_name) > 0) {
    // detail view
    if (button1->is_fresh() && button1->get_data().shortPress) {
      force_next_render();
      if (!_detail_log_upload_id) {
        // Upload
        upload_detail(controller.get_settings());
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      // Back: detail -> list
      force_next_render();
      leave_detail();
      return nullptr;
    }
  } else if (_current_view == e_log_main_view) {
    // category selector: B1=Down, B2=Back, B3=Select
    if (button1->is_fresh() && button1->get_data().shortPress) {
      force_next_render();
      _main_selected_index = (_main_selected_index + 1) % MAIN_VIEW_ITEM_COUNT;
      return nullptr;
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // Back: main -> menu
      return &MenuWindow_i;
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      force_next_render();
      switch (_main_selected_index) {
        case 0: _current_view = e_log_drive_view;   break;
        case 1: _current_view = e_log_survey_view;  break;
        case 2: _current_view = e_log_flight_view;  break;
        case 3: _current_view = e_log_journal_view; break;
      }
      load_log_list(current_dir());
      return nullptr;
    }
  } else {
    // file list view: B1=Down, B2=Back, B3=Select
    if (button1->is_fresh() && button1->get_data().shortPress) {
      force_next_render();
      if (_file_count > 0) {
        _selected_index = (_selected_index + 1) % _file_count;
      }
      return nullptr;
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // Back: list -> main
      force_next_render();
      _current_view = e_log_main_view;
      _selected_index = 0;
      _file_count = 0;
      return nullptr;
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      force_next_render();
      if (_file_count == 0) {
        return nullptr;
      }
      char full_path[LOG_FILENAME_SIZE];
      snprintf(full_path, LOG_FILENAME_SIZE, "%s/%s",
               current_dir(), _file_list[_selected_index]);
      enter_detail(full_path);
      return nullptr;
    }
  }

  return nullptr;
}

void LogViewerScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }

  clear_screen_content();

  if (strlen(_detail_log_file_name) > 0) {
    return render_log_detail(workers, handlers, force);
  }

  switch (_current_view) {
    case e_log_main_view:
      return render_main(workers, handlers, force);
    case e_log_drive_view:
    case e_log_survey_view:
    case e_log_journal_view:
      return render_log_list(current_dir(), workers, handlers, force);
    default:
      return;
  }
}

void LogViewerScreen::render_main(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("Down");
  drawButton2("Back");
  drawButton3("Select");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  const char* labels[MAIN_VIEW_ITEM_COUNT] = {"Drive logs", "Survey logs", "Flight logs", "Journal logs"};
  for (int i = 0; i < MAIN_VIEW_ITEM_COUNT; i++) {
    M5.Lcd.printf("%s  %s\n", (i == _main_selected_index) ? ">" : " ", labels[i]);
  }
}

void LogViewerScreen::render_log_list(const char* dir, const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("Down");
  drawButton2("Back");
  drawButton3("Select");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  if (_file_count == 0) {
    M5.Lcd.printf("%s/\n", dir ? dir : "");
    M5.Lcd.printf("   (no logs)\n");
    return;
  }

  const int total_pages = (_file_count + LOG_LIST_PAGE - 1) / LOG_LIST_PAGE;
  const int page = _selected_index / LOG_LIST_PAGE;
  const int start = page * LOG_LIST_PAGE;
  const int end = (start + LOG_LIST_PAGE < _file_count) ? start + LOG_LIST_PAGE : _file_count;
  M5.Lcd.printf("%s/  (%d/%d)\n", dir ? dir : "", page + 1, total_pages);
  for (int i = start; i < end; i++) {
    M5.Lcd.printf("%s  %s\n",
                  (i == _selected_index) ? ">" : " ",
                  _file_list[i]);
  }
}

void LogViewerScreen::render_log_detail(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  const bool can_upload = !_detail_log_upload_id
                          && WiFiWrapper_i.wifi_connected()
                          && _upload_status != e_upload_in_progress;
  drawButton1("Upload", can_upload ? e_button_default : e_button_disabled);
  drawButton2("");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  M5.Lcd.printf("Log name:      %s\n", _detail_log_file_name);
  M5.Lcd.printf("Timestamp:     %04hu/%02hhu/%02hhu %02hhu:%02hhu\n", _detail_log_timestamp.year, _detail_log_timestamp.month, _detail_log_timestamp.day, _detail_log_timestamp.hour, _detail_log_timestamp.minute);
  M5.Lcd.printf("WiFi:          %s\n", WiFiWrapper_i.wifi_connected() ? "connected" : "connecting...");

  switch (_upload_status) {
    case e_upload_idle:
      M5.Lcd.printf("Status:        ready\n");
      break;
    case e_upload_in_progress:
      M5.Lcd.printf("Status:        uploading...\n");
      break;
    case e_upload_success:
      M5.Lcd.printf("Status:        OK (HTTP %d)\n", _upload_http_code);
      break;
    case e_upload_failed:
      M5.Lcd.printf("Status:        FAILED (HTTP %d)\n", _upload_http_code);
      break;
  }
  if (_detail_log_upload_id) {
    M5.Lcd.printf("Upload log id: %u\n", _detail_log_upload_id);
  }
}

void LogViewerScreen::enter_detail(const char* log_name) {
  char dir[100];
  strcpy(_detail_log_file_path, log_name);
  if (sscanf(_detail_log_file_path, "/%[^/]/%s", dir, _detail_log_file_name) == 0) {
    strcpy(_detail_log_file_name, "Unknown log name");
    return;
  }
  if (sscanf(_detail_log_file_name, "%4hu-%2hhu-%2hhu_%2hhu%2hhu.log", &_detail_log_timestamp.year, &_detail_log_timestamp.month, &_detail_log_timestamp.day, &_detail_log_timestamp.hour, &_detail_log_timestamp.minute) != 5) {
    _detail_log_timestamp.clear();
  }

  // TODO: parse log_name
}

void LogViewerScreen::leave_detail() {
  strcpy(_detail_log_file_name, "");
  _detail_log_timestamp.clear();
  _detail_upload_timestamp.clear();
  _detail_log_upload_id = 0;
  _upload_status = e_upload_idle;
  _upload_http_code = 0;
}

void LogViewerScreen::enter_screen(Controller& controller) {
  leave_detail();
  _current_view = e_log_main_view;
  _main_selected_index = 0;
  _selected_index = 0;
  _file_count = 0;
  // Use the active WiFi profile (consistent with fixed_mode / api_connector).
  // first_time=true to force a fresh begin() rather than a passive reconnect,
  // since other screens may have soft-disconnected the radio.
  const auto& s = controller.get_settings();
  WiFiWrapper_i.connect_wifi(s.get_active_wifi_ssid(), s.get_active_wifi_password(), true);
}

void LogViewerScreen::leave_screen(Controller& controller) {
  // Do not disconnect on leave: api_connector / fixed_mode etc. manage their own
  // lifecycle and will turn WiFi off when appropriate. Leaving WiFi up reduces
  // reconnect churn on Core2 where disconnect/reconnect cycles cost seconds.
}

void LogViewerScreen::upload_detail(const LocalStorage& config) {
  _upload_status = e_upload_in_progress;
  _upload_http_code = 0;
  // Repaint the Status line in place so the user sees feedback before we block.
  // GFXScreen::loop() draws every frame inside setRotation(3)..setRotation(1) and
  // leaves the LCD at rotation 1 between ticks. We're called from handle_input,
  // outside that envelope, so we must re-establish rotation 3 or the text renders
  // 180-degrees flipped relative to the rest of the screen.
  // Status line is the 4th Font2 row of render_log_detail (y=40 + 16*3 = 88);
  // clear it first so leftover glyphs from "ready" don't overlap "uploading...".
  M5.Lcd.startWrite();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillRect(0, 88, M5.Lcd.width(), 16, LCD_COLOR_BACKGROUND);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 88, &fonts::Font2);
  M5.Lcd.printf("Status:        uploading...");
  M5.Lcd.setRotation(1);
  M5.Lcd.display();
  M5.Lcd.endWrite();

  File log_file = SDInterface::i().get_file(_detail_log_file_path);

  if (!log_file) {
    M5_LOGD("Failed to open log file!");
    _upload_status = e_upload_failed;
    return;
  }

  char url[160];
  const char* endpoint = (config.get_api_logfile_dest() == 1 && strlen(config.get_api_logfile_endpoint_2()) > 0)
      ? config.get_api_logfile_endpoint_2()
      : API_LOGFILE_ENDPOINT;
  snprintf(url, sizeof(url), "%s?api_key=%s", endpoint, config.get_api_key());

  ChunkedHTTPClient http;
  WiFiClientSecure client;
  // TODO: pin Safecast CA bundle. Using setInsecure() until cert plumbing lands.
  client.setInsecure();

  // Specify destination for HTTPS request
  if (!http.begin(client, url)) {
    M5_LOGD("Unable to begin URL connection");
    http.end(); // Free resources
    log_file.close();
    _upload_status = e_upload_failed;
    return;
  }

  // Define boundary and headers
  char boundary[32];
  sprintf(boundary, "----WebKitFormBoundaryZen%d", config.get_device_id());
  char content_type_header[150];
  sprintf(content_type_header, "multipart/form-data; boundary=%s", boundary);

  http.setUserAgent(HEADER_API_USER_AGENT);
  // HTTPClient sets Host: from the URL; do not duplicate it here.
  http.addHeader("Content-Type", content_type_header);
  http.addHeader("Transfer-Encoding", "chunked");
  http.addHeader("Accept", "application/json");

  char chunk_header[16];
  static constexpr size_t CHUNK_BUF_SZ = 1024;
  char chunk_buffer[CHUNK_BUF_SZ];
  size_t chunk_length;

  if (http.sendPOSTHeaders()) {
    // 1. Send the opening boundary and metadata for the file part as a chunk
    // RFC 7578: each part = boundary line, headers, blank line, body, CRLF before next boundary.
    sprintf(chunk_buffer,
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"bgeigie_import[description]\"\r\n"
            "\r\n"
            "Uploaded from Zen\r\n"
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"bgeigie_import[source]\"; filename=\"%s\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n",
            boundary, boundary, _detail_log_file_name);

    // Calculate header length
    chunk_length = strlen(chunk_buffer);
    sprintf(chunk_header, "%X\r\n", chunk_length); // Chunk size in hexadecimal
    client.print(chunk_header);              // Send chunk size
    client.print(chunk_buffer);   // Send chunk data
    client.print("\r\n");                                   // End of chunk
    M5_LOGD("Header chunk sent (%X):\n%s", chunk_length, chunk_buffer);

    // 2. Stream the file content in fixed-size blocks. One TLS record per ~1 KB
    //    keeps throughput reasonable (vs one record per line). yield() after every
    //    chunk so we don't starve the WiFi task or trip the task watchdog.
    const uint32_t total_bytes = log_file.size();
    uint32_t sent_bytes = 0;
    uint32_t last_log = 0;
    while (log_file.available()) {
      int n = log_file.read((uint8_t*)chunk_buffer, CHUNK_BUF_SZ);
      if (n <= 0) break;
      sprintf(chunk_header, "%X\r\n", n);
      client.write((const uint8_t*)chunk_header, strlen(chunk_header));
      client.write((const uint8_t*)chunk_buffer, n);
      client.write((const uint8_t*)"\r\n", 2);
      sent_bytes += n;
      if (sent_bytes - last_log >= 8192) {
        M5_LOGD("Upload progress: %u / %u bytes", sent_bytes, total_bytes);
        last_log = sent_bytes;
      }
      yield();
    }
    M5_LOGD("Upload streaming complete: %u bytes", sent_bytes);

    // 3. Send the closing boundary as a chunk (with CRLF prefix to terminate file body)
    sprintf(chunk_buffer, "\r\n--%s--\r\n", boundary);
    chunk_length = strlen(chunk_buffer);
    sprintf(chunk_header, "%X\r\n", chunk_length); // Chunk size in hexadecimal
    client.print(chunk_header);                              // Send chunk size
    client.print(chunk_buffer);                          // Send chunk data
    client.print("\r\n");                                   // End of chunk
    M5_LOGD("Closing boundary chunk sent (%X):\n%s", chunk_length, chunk_buffer);

    // 4. End the chunked transfer by sending a zero-size chunk
    client.print("0\r\n\r\n");
    M5_LOGD("End of chunked transfer");

    // 5. Wait for the server response
    int statusCode = http.handleResponse();
    String response = http.getString();
    M5_LOGD("POST complete, (code %d) response:\n%s", statusCode, response.c_str());

    _upload_http_code = statusCode;
    if (statusCode >= 200 && statusCode < 300) {
      _upload_status = e_upload_success;
      // Best-effort parse: bgeigie_imports returns {"id":<n>, ...}
      int id_idx = response.indexOf("\"id\":");
      if (id_idx >= 0) {
        _detail_log_upload_id = (uint32_t) strtoul(response.c_str() + id_idx + 5, nullptr, 10);
      }
    } else {
      _upload_status = e_upload_failed;
    }

    WiFiWrapper_i.update_active();
  } else {
    M5_LOGD("Failed to initiate request");
    _upload_status = e_upload_failed;
  }

  http.end(); // Free resources
  log_file.close(); // Close file
  force_next_render();
}
