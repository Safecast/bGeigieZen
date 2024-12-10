#ifndef SCREENS_LOG_VIEWER_H
#define SCREENS_LOG_VIEWER_H

#include "base_screen.h"


class LogViewerScreen : public BaseScreen {
 public:

  explicit LogViewerScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  enum LogView {
    e_log_main_view,
    e_log_journal_view,
    e_log_drive_view,
    e_log_survey_view,
  };

  void enter_detail(const char* log_name);
  void leave_detail();

  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;
  void render_main(const worker_map_t& workers, const handler_map_t& handlers, bool force);
  void render_journal_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force);
  void render_drive_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force);
  void render_survey_log_list(const worker_map_t& workers, const handler_map_t& handlers, bool force);
  void render_log_detail(const worker_map_t& workers, const handler_map_t& handlers, bool force);

  LogView _current_view;
  char _detail_log_file_name[LOG_FILENAME_SIZE];
  char _detail_log_timestamp[20];
  char _detail_log_uploaded_timestamp[20];
  uint32_t _detail_log_uploaded_id;

};

extern LogViewerScreen LogViewerScreen_i;

#endif //SCREENS_LOG_VIEWER_H
