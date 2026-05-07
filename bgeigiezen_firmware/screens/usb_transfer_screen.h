#ifndef BGEIGIEZEN_USB_TRANSFER_SCREEN_H_
#define BGEIGIEZEN_USB_TRANSFER_SCREEN_H_

#include <M5Unified.hpp>
#include "base_screen.h"
#include "controller.h"
#include "workers/zen_button.h"
#include "utils/sd_wrapper.h"

/**
 * USB File Transfer Screen - Provides SD card to USB-C file transfer functionality
 * Allows users to access SD card files through USB connection
 */
class USBTransferScreen : public BaseScreen {
 public:
  enum TransferState {
    e_idle,
    e_enabling_usb,
    e_usb_active,
    e_disabling_usb,
    e_error
  };

  USBTransferScreen();
  virtual ~USBTransferScreen() = default;

  // BaseScreen interface
  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 private:
  // UI helpers
  void drawStatusInfo();
  void drawUSBStatus();
  void drawTransferProgress();
  void drawButtonLabels();
  
  // USB functionality
  bool enableUSBStorage(Controller& controller);
  bool disableUSBStorage();
  void updateUSBState();
  
  // Utility functions
  void formatBytes(uint64_t bytes, char* buffer, size_t bufferSize);
  uint32_t countSDCardFiles();
  void updateSDCardInfo();
  uint32_t countFilesRecursive(File dir);
  
  // SD Card cleanup for USB MSC
  void stopAllSDCardLoggers(Controller& controller);

  // State management
  TransferState _transfer_state;
  uint32_t _last_state_change;
  uint32_t _last_progress_update;
  uint32_t _transfer_progress;
  char _status_message[100];
  char _usb_device_name[50];
  uint64_t _sd_total_space;
  uint64_t _sd_free_space;
  uint32_t _sd_file_count;
  bool _usb_connected;
  bool _usb_available;

  // Captured at enter_screen so enable/disable callbacks can stop loggers.
  Controller* _controller;
  
  static constexpr uint32_t USB_STATE_CHECK_INTERVAL = 1000; // 1 second
  static constexpr uint32_t MAX_STATUS_MESSAGE_LENGTH = 100;
};

// Global instance
extern USBTransferScreen USBTransferScreen_i;

#endif // BGEIGIEZEN_USB_TRANSFER_SCREEN_H_
