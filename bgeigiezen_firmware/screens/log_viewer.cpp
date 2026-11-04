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
#include "utils/log_upload.h"


LogViewerScreen LogViewerScreen_i;


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

  uint32_t upload_id = 0;
  _upload_http_code = upload_log_file(_detail_log_file_path, _detail_log_file_name, config, &upload_id);
  if (_upload_http_code >= 200 && _upload_http_code < 300) {
    _upload_status = e_upload_success;
    _detail_log_upload_id = upload_id;
  } else {
    _upload_status = e_upload_failed;
  }
  force_next_render();
}
