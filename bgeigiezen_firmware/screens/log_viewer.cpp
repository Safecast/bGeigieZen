#include "log_viewer.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"


LogViewerScreen LogViewerScreen_i;

LogViewerScreen::LogViewerScreen() : BaseScreen("Log view", true),
                                     _current_view(e_log_main_view),
                                     _detail_log_file_name(""),
                                     _detail_log_timestamp(""),
                                     _detail_log_uploaded_timestamp(""),
                                     _detail_log_uploaded_id(0) {
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
      leave_detail();
      return nullptr;
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      if (!_detail_log_uploaded_id && !strlen(_detail_log_uploaded_timestamp)) {
        // Upload
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      return &MenuWindow_i;
    }
  } else {
    // not detail view
    if (button1->is_fresh() && button1->get_data().shortPress) {
      // TODO: selector down
      return nullptr;
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // TODO: enter detail down
      enter_detail("journal/test_log.txt");
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      return &MenuWindow_i;
    }
  }

  return nullptr;
}

void LogViewerScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
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
}

void LogViewerScreen::render_journal_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
}

void LogViewerScreen::render_drive_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
}

void LogViewerScreen::render_survey_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
}

void LogViewerScreen::render_log_detail(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
}

void LogViewerScreen::enter_detail(const char* log_name) {
  strcpy(_detail_log_file_name, log_name);
  // TODO: parse log_name
  strcpy(_detail_log_timestamp, "");
  strcpy(_detail_log_uploaded_timestamp, "");
  _detail_log_uploaded_id = 0;
}

void LogViewerScreen::leave_detail() {
  strcpy(_detail_log_file_name, "");
  strcpy(_detail_log_timestamp, "");
  strcpy(_detail_log_uploaded_timestamp, "");
  _detail_log_uploaded_id = 0;
}

void LogViewerScreen::enter_screen(Controller& controller) {
  leave_detail();
  _current_view = e_log_main_view;
}

void LogViewerScreen::leave_screen(Controller& controller) {
}
