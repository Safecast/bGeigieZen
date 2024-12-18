#include <HTTPClient.h>

#include "log_viewer.h"
#include "identifiers.h"
#include "menu_window.h"
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
                                     _detail_log_upload_id(0) {
  required_wifi = true;
  required_sd = true;
}

BaseScreen* LogViewerScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);


  if (strlen(_detail_log_file_name) > 0) {
    // detail view
    if (button1->is_fresh() && button1->get_data().shortPress) {
      // Back
      force_next_render();
      leave_detail();
      return nullptr;
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      force_next_render();
      if (!_detail_log_upload_id) {
        // Upload
        upload_detail(controller.get_settings());
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      return &MenuWindow_i;
    }
  } else {
    // not detail view
    if (button1->is_fresh() && button1->get_data().shortPress) {
      force_next_render();
      // TODO: selector down
      return nullptr;
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      if (_current_view == e_log_main_view) {
        force_next_render();
        _current_view = e_log_drive_view;
      } else {
        force_next_render();
        // TODO: enter detail down
        enter_detail("/drives/2024-11-30_1227.log");
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      return &MenuWindow_i;
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
    case e_log_journal_view:
      return render_journal_log_list(workers, handlers, force);
    case e_log_drive_view:
      return render_drive_log_list(workers, handlers, force);
    case e_log_survey_view:
      return render_survey_log_list(workers, handlers, force);
    default:
      return;
  }
}

void LogViewerScreen::render_main(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("Down");
  drawButton2("Select");
  drawButton3("Menu");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  M5.Lcd.printf(">  Drive logs\n");
  M5.Lcd.printf("   Survey logs\n");
  M5.Lcd.printf("   Journal logs\n");
}

void LogViewerScreen::render_journal_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
}

void LogViewerScreen::render_drive_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("Down");
  drawButton2("Select");
  drawButton3("Menu");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  M5.Lcd.printf("   Back\n");
  M5.Lcd.printf(">  2024-11-30_1227.log\n");
  M5.Lcd.printf("   test2.log\n");
  M5.Lcd.printf("   test3.log\n");
}

void LogViewerScreen::render_survey_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
}

void LogViewerScreen::render_log_detail(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("Back");
  drawButton2("Upload", _detail_log_upload_id || !WiFiWrapper_i.wifi_connected() ? e_button_disabled : e_button_default);
  drawButton3("Menu");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  M5.Lcd.printf("Log name:      %s\n", _detail_log_file_name);
  M5.Lcd.printf("Timestamp:     %04hu/%02hhu/%02hhu %02hhu:%02hhu\n", _detail_log_timestamp.year, _detail_log_timestamp.month, _detail_log_timestamp.day, _detail_log_timestamp.hour, _detail_log_timestamp.minute);
  M5.Lcd.printf("Uploaded on:   -\n");
  M5.Lcd.printf("Upload log id: -\n\n");
  M5.Lcd.printf("QR code here\n");


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
}

void LogViewerScreen::enter_screen(Controller& controller) {
  leave_detail();
  _current_view = e_log_main_view;
  WiFiWrapper_i.connect_wifi(controller.get_settings().get_wifi_ssid(), controller.get_settings().get_wifi_password());
}

void LogViewerScreen::leave_screen(Controller& controller) {
  WiFiWrapper_i.disconnect_wifi();
}

void LogViewerScreen::upload_detail(const LocalStorage& config) {
  File log_file = SDInterface::i().get_file(_detail_log_file_path);

  if (!log_file) {
    M5_LOGD("Failed to open log file!");
    return;
  }

  char url[100];
  sprintf(url, "%s?api_key=%s&", API_LOGFILE_ENDPOINT, config.get_api_key());

  ChunkedHTTPClient http;
  WiFiClient client;

  // Specify destination for HTTP request
  if (!http.begin(client, url)) {
    M5_LOGD("Unable to begin URL connection");
    http.end(); // Free resources
    return;
  }

  // Define boundary and headers
  char boundary[32];
  sprintf(boundary, "----WebKitFormBoundaryZen%d", config.get_device_id());
  char content_type_header[150];
  sprintf(content_type_header, "multipart/form-data; boundary=%s", boundary);

  http.setUserAgent(HEADER_API_USER_AGENT);
  http.addHeader("Host", TTSERVE_HOST);
  http.addHeader("Content-Type", content_type_header);
  http.addHeader("Transfer-Encoding", "chunked");

  char chunk_header[16];
  char chunk_buffer[512];
  size_t chunk_length;


  if (http.sendPOSTHeaders()) {
    // 1. Send the opening boundary and metadata for the file part as a chunk
    sprintf(chunk_buffer,
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"bgeigie_import[description]\"\r\n"
            "Uploaded from Zen\r\n\r\n"
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"bgeigie_import[source]\"; filename=\"%s\"\r\n"
            "Content-Type: text/plain\r\n\r\n",
            boundary, boundary, "2024-11-30_1227.log");

    // Calculate header length
    chunk_length = strlen(chunk_buffer);
    sprintf(chunk_header, "%X\r\n", chunk_length); // Chunk size in hexadecimal
    client.print(chunk_header);              // Send chunk size
    client.print(chunk_buffer);   // Send chunk data
    client.print("\r\n");                                   // End of chunk
    M5_LOGD("Header chunk sent (%X):\n%s", chunk_length, chunk_buffer);

    // 2. Stream the file content line by line as chunks
    while (log_file.available()) {
      chunk_length = log_file.readBytesUntil('\n', chunk_buffer, 511); // Read a line from the file
      chunk_buffer[chunk_length] = '\n'; // Add the newline character back
      chunk_buffer[chunk_length + 1] = '\0'; // Null-terminate the buffer

      sprintf(chunk_header, " %X\r\n", chunk_length + 1); // Chunk size in hexadecimal (include '\n')
      client.print(chunk_header);              // Send chunk size
      client.print(chunk_buffer);               // Send chunk data
      client.print("\r\n");                                   // End of chunk
    }

    // 3. Send the closing boundary as a chunk
    sprintf(chunk_buffer, "--%s--\r\n", boundary);
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

    WiFiWrapper_i.update_active();
  } else {
    M5_LOGD("Failed to initiate request");
  }

  http.end(); // Free resources


  log_file.close(); // Close file

}
