#include "usb_transfer_screen.h"
#include "menu_window.h"
#include "identifiers.h"
#include "user_config.h"
#include <M5Unified.hpp>
#include <SD.h>

// USB MSC requires the ESP32-S3 TinyUSB stack. On classic ESP32 (Core/Core2)
// these headers exist but compile out via CONFIG_TINYUSB_*_ENABLED, so the
// USBMSC class is empty and we fall back to a "not supported" UI.
#include "sdkconfig.h"
#if CONFIG_TINYUSB_MSC_ENABLED && !ARDUINO_USB_MODE
  #define USB_TRANSFER_HAS_MSC 1
  #include "USB.h"
  #include "USBMSC.h"
#else
  #define USB_TRANSFER_HAS_MSC 0
#endif

// The project gates M5_LOG* behind CORE_DEBUG_LEVEL (defaults to ERROR-only),
// so these diagnostics use Serial.printf directly to match the convention
// established by PowerMgr — that way they always show in the monitor.
// Defined unconditionally so non-MSC builds (Core1/Core2) can still log.
#define USB_LOG(fmt, ...)  Serial.printf("[USB-MSC] " fmt "\r\n", ##__VA_ARGS__)

#if USB_TRANSFER_HAS_MSC
// Constructed at static-init time so the MSC interface is registered before
// app_main() runs USB.begin(). Once TinyUSB is up, MSC stays in the descriptor
// for the rest of the boot — we just toggle mediaPresent() to "insert" or
// "eject" the SD card on demand.
static USBMSC g_usb_msc;
static bool g_usb_started = false;
static uint32_t g_msc_read_count = 0;
static uint32_t g_msc_write_count = 0;
static uint32_t g_msc_read_fail_count = 0;
static uint32_t g_msc_write_fail_count = 0;

static int32_t usb_msc_read_cb(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  // SDFS::readRAW reads exactly one 512-byte sector. The host can request
  // larger transfers, so loop until the buffer is full.
  const uint16_t sector_size = SD.sectorSize();
  if (sector_size == 0 || (bufsize % sector_size) != 0) {
    USB_LOG("read: bad geometry sector_size=%u bufsize=%u", sector_size, bufsize);
    return -1;
  }
  uint32_t sectors = bufsize / sector_size;
  uint8_t* dst = static_cast<uint8_t*>(buffer);
  for (uint32_t i = 0; i < sectors; ++i) {
    if (!SD.readRAW(dst + i * sector_size, lba + i)) {
      ++g_msc_read_fail_count;
      USB_LOG("read FAIL lba=%u (chunk %u/%u, offset=%u, bufsize=%u)",
              lba + i, i + 1, sectors, offset, bufsize);
      return -1;
    }
  }
  uint32_t n = ++g_msc_read_count;
  if (n <= 8 || (n % 512) == 0) {
    USB_LOG("read #%u lba=%u sectors=%u", n, lba, sectors);
  }
  return bufsize;
}

static int32_t usb_msc_write_cb(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  const uint16_t sector_size = SD.sectorSize();
  if (sector_size == 0 || (bufsize % sector_size) != 0) {
    USB_LOG("write: bad geometry sector_size=%u bufsize=%u", sector_size, bufsize);
    return -1;
  }
  uint32_t sectors = bufsize / sector_size;
  for (uint32_t i = 0; i < sectors; ++i) {
    if (!SD.writeRAW(buffer + i * sector_size, lba + i)) {
      ++g_msc_write_fail_count;
      USB_LOG("write FAIL lba=%u (chunk %u/%u, offset=%u, bufsize=%u)",
              lba + i, i + 1, sectors, offset, bufsize);
      return -1;
    }
  }
  uint32_t n = ++g_msc_write_count;
  if (n <= 8 || (n % 512) == 0) {
    USB_LOG("write #%u lba=%u sectors=%u", n, lba, sectors);
  }
  return bufsize;
}

static bool usb_msc_start_stop_cb(uint8_t /*power_condition*/, bool /*start*/, bool load_eject) {
  // Host triggered an eject; respect it by clearing media-present.
  if (load_eject) {
    g_usb_msc.mediaPresent(false);
  }
  return true;
}
#endif  // USB_TRANSFER_HAS_MSC

USBTransferScreen USBTransferScreen_i;

USBTransferScreen::USBTransferScreen() : BaseScreen("USB Transfer", true),
                                         _transfer_state(e_idle),
                                         _last_state_change(0),
                                         _last_progress_update(0),
                                         _transfer_progress(0),
                                         _sd_total_space(0),
                                         _sd_free_space(0),
                                         _sd_file_count(0),
                                         _usb_connected(false),
                                         _usb_available(false),
                                         _controller(nullptr) {
  // We deliberately do NOT set required_sd = true here. The framework would
  // overlay "NO SD CARD INSERTED" (and turn the SD status icon red) the
  // moment we lock SDInterface for MSC, because can_write_logs() returns
  // false while locked. We handle the SD-missing case ourselves in
  // enter_screen() with a clearer message.
  required_sd = false;

  _status_message[0] = '\0';
  _usb_device_name[0] = '\0';
  snprintf(_usb_device_name, sizeof(_usb_device_name), "bGeigieZen Storage");
}

BaseScreen* USBTransferScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (button1->is_fresh() && button1->get_data().shortPress) {
    if (_transfer_state == e_usb_active) {
      disableUSBStorage();
    } else if (_transfer_state == e_idle || _transfer_state == e_error) {
      enableUSBStorage(controller);
    }
    force_next_render();
    return nullptr;
  }

  if (button2->is_fresh() && button2->get_data().shortPress) {
    updateSDCardInfo();
    force_next_render();
    return nullptr;
  }

  if (button3->is_fresh() && button3->get_data().shortPress) {
    return &MenuWindow_i;
  }

  return nullptr;
}

void USBTransferScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  // The framework calls render() every LCD_REFRESH_RATE tick (and on worker
  // updates), but with force=false unless we ask. Drive the state machine on
  // every tick regardless of force, and request a forced redraw whenever
  // state or progress changes — otherwise the transition timers stall and
  // the progress bar never moves.
  uint32_t current_time = millis();
  uint32_t update_interval =
      (_transfer_state == e_enabling_usb || _transfer_state == e_disabling_usb) ? 100 : USB_STATE_CHECK_INTERVAL;
  if (current_time - _last_progress_update >= update_interval) {
    TransferState prev_state = _transfer_state;
    uint32_t prev_progress = _transfer_progress;
    updateUSBState();
    _last_progress_update = current_time;
    if (_transfer_state != prev_state || _transfer_progress != prev_progress) {
      force = true;
    }
  }
  if (_transfer_state == e_enabling_usb || _transfer_state == e_disabling_usb) {
    // Keep the redraw loop pumping while we animate the progress bar.
    force_next_render();
  }

  if (!force) {
    return;
  }

  clear_screen_content();
  drawStatusInfo();
  drawUSBStatus();
  drawTransferProgress();
  drawButtonLabels();
}

void USBTransferScreen::enter_screen(Controller& controller) {
  USB_LOG("Entering USB Transfer screen");
  _controller = &controller;

  _transfer_state = e_idle;
  _last_state_change = millis();
  _last_progress_update = millis();
  _transfer_progress = 0;
  _usb_connected = false;
  _usb_available = false;

#if !USB_TRANSFER_HAS_MSC
  _transfer_state = e_error;
  strncpy(_status_message, "USB MSC not supported on this board", sizeof(_status_message));
  USB_LOG("USB MSC unavailable: TinyUSB not enabled or wrong USB mode");
#else
  if (!SDInterface::i().ready()) {
    _transfer_state = e_error;
    strncpy(_status_message, "NO SD CARD INSERTED", sizeof(_status_message));
  } else {
    updateSDCardInfo();
    strncpy(_status_message, "Ready to enable USB storage", sizeof(_status_message));
  }
#endif

  force_next_render();
}

void USBTransferScreen::leave_screen(Controller& controller) {
  USB_LOG("Leaving USB Transfer screen");
  if (_transfer_state == e_usb_active || _transfer_state == e_enabling_usb) {
    disableUSBStorage();
  } else if (SDInterface::i().is_locked_for_msc()) {
    // Defensive: if state machine ended up out of sync, make sure we never
    // leave the SD card locked — that would silently kill all logging.
    SDInterface::i().unlock_for_msc();
    SDInterface::i().force_remount();
  }
  _controller = nullptr;
}

void USBTransferScreen::drawStatusInfo() {
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.setTextFont(2);

  if (_transfer_state == e_error) {
    M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%s\n", _status_message);
  } else {
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    char totalStr[20], freeStr[20];
    formatBytes(_sd_total_space, totalStr, sizeof(totalStr));
    formatBytes(_sd_free_space, freeStr, sizeof(freeStr));
    M5.Lcd.printf("SD Card Space:\n");
    M5.Lcd.printf("  Total: %s\n", totalStr);
    M5.Lcd.printf("  Free:  %s\n", freeStr);
    M5.Lcd.printf("  Files: %lu\n", _sd_file_count);
  }

  M5.Lcd.drawLine(0, 100, 320, 100, LCD_COLOR_DEFAULT);
}

void USBTransferScreen::drawUSBStatus() {
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 110);
  M5.Lcd.setTextFont(2);

  M5.Lcd.printf("USB Status:\n");

  switch (_transfer_state) {
    case e_idle:
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("  Ready to enable USB storage\n");
      M5.Lcd.printf("  Device: %s\n", _usb_device_name);
      break;
    case e_enabling_usb:
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("  Enabling USB storage...\n");
      break;
    case e_usb_active:
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("  USB storage ACTIVE\n");
      M5.Lcd.printf("  Connect USB-C cable to computer\n");
      M5.Lcd.printf("  Files accessible via file browser\n");
      break;
    case e_disabling_usb:
      M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("  Disabling USB storage...\n");
      break;
    case e_error:
      M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("  Error: %s\n", _status_message);
      break;
  }
}

void USBTransferScreen::drawTransferProgress() {
  if (_transfer_state == e_enabling_usb || _transfer_state == e_disabling_usb) {
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.setCursor(0, 160);
    M5.Lcd.setTextFont(2);
    M5.Lcd.printf("Progress: %lu%%\n", _transfer_progress);

    int bar_width = 280;
    int bar_height = 10;
    int bar_x = 20;
    int bar_y = 180;
    M5.Lcd.drawRect(bar_x, bar_y, bar_width, bar_height, LCD_COLOR_DEFAULT);
    int progress_width = (bar_width - 2) * _transfer_progress / 100;
    M5.Lcd.fillRect(bar_x + 1, bar_y + 1, progress_width, bar_height - 2, LCD_COLOR_ACTIVITY);
  }
}

void USBTransferScreen::drawButtonLabels() {
  switch (_transfer_state) {
    case e_idle:
      drawButton1("Enable USB");
      break;
    case e_error:
#if USB_TRANSFER_HAS_MSC
      drawButton1("Enable USB");
#else
      drawButton1("Unavailable", BaseScreen::e_button_disabled);
#endif
      break;
    case e_usb_active:
      drawButton1("Disable USB");
      break;
    case e_enabling_usb:
    case e_disabling_usb:
      drawButton1("Please Wait", BaseScreen::e_button_disabled);
      break;
  }

  drawButton2("Refresh");
  drawButton3("Menu");
}

void USBTransferScreen::stopAllSDCardLoggers(Controller& controller) {
  controller.set_handler_active(k_handler_journal_logger, false);
  controller.set_handler_active(k_handler_drive_logger, false);
  controller.set_handler_active(k_handler_survey_logger, false);
  controller.set_handler_active(k_handler_flight_logger, false);
}

bool USBTransferScreen::enableUSBStorage(Controller& controller) {
  USB_LOG("Attempting to enable USB storage mode");

  if (_transfer_state != e_idle && _transfer_state != e_error) {
    return false;
  }

#if !USB_TRANSFER_HAS_MSC
  _transfer_state = e_error;
  strncpy(_status_message, "USB MSC not supported on this board", sizeof(_status_message));
  return false;
#else
  if (!SDInterface::i().ready()) {
    _transfer_state = e_error;
    strncpy(_status_message, "No SD card available", sizeof(_status_message));
    return false;
  }

  // Capture geometry while the FAT mount is still live, then explicitly stop
  // loggers and lock SDInterface so nothing on the main loop task issues
  // FATFS calls while the TinyUSB task drives raw block I/O on the same SPI
  // bus (sd_diskio has no mutex protection).
  uint32_t num_sectors = SD.numSectors();
  uint16_t sector_size = SD.sectorSize();
  uint64_t card_size_bytes = SD.cardSize();
  uint64_t total_bytes_fat = SD.totalBytes();
  uint8_t card_type = SD.cardType();
  USB_LOG("SD geometry: cardType=%u numSectors=%u sectorSize=%u cardSize=%llu bytes totalBytes(FAT)=%llu",
          card_type, num_sectors, sector_size, card_size_bytes, total_bytes_fat);
  if (num_sectors == 0 || sector_size == 0) {
    _transfer_state = e_error;
    strncpy(_status_message, "Could not read SD geometry", sizeof(_status_message));
    USB_LOG("ERROR: SD numSectors/sectorSize returned 0");
    return false;
  }

  stopAllSDCardLoggers(controller);
  SDInterface::i().lock_for_msc();

  USB_LOG("Starting USB MSC: %u sectors x %u bytes (%llu MB)",
          num_sectors, sector_size, ((uint64_t)num_sectors * sector_size) / (1024ULL * 1024ULL));

  g_usb_msc.vendorID("Safecast");
  g_usb_msc.productID("bGeigieZen");
  g_usb_msc.productRevision("1.0");
  g_usb_msc.onRead(usb_msc_read_cb);
  g_usb_msc.onWrite(usb_msc_write_cb);
  g_usb_msc.onStartStop(usb_msc_start_stop_cb);

  if (!g_usb_msc.begin(num_sectors, sector_size)) {
    _transfer_state = e_error;
    strncpy(_status_message, "USBMSC.begin() failed", sizeof(_status_message));
    USB_LOG("ERROR: USBMSC.begin() failed");
    return false;
  }
  g_usb_msc.mediaPresent(true);

  // USB.begin() is one-shot in arduino-esp32 — with CDC-on-boot it has
  // already been called by app_main(), so this is just a safety net for
  // builds without CDC-on-boot.
  if (!g_usb_started) {
    USB.productName("bGeigieZen Storage");
    USB.manufacturerName("Safecast");
    USB.begin();
    g_usb_started = true;
  }

  _transfer_state = e_enabling_usb;
  _last_state_change = millis();
  _last_progress_update = millis();
  _transfer_progress = 0;
  strncpy(_status_message, "Mounting on host...", sizeof(_status_message));
  return true;
#endif
}

bool USBTransferScreen::disableUSBStorage() {
  USB_LOG("Attempting to disable USB storage mode");

  if (_transfer_state != e_usb_active && _transfer_state != e_enabling_usb) {
    return false;
  }

#if USB_TRANSFER_HAS_MSC
  // Eject from the host's perspective and detach our callbacks. We can't
  // un-register the MSC interface or end TinyUSB without rebooting, but
  // mediaPresent(false) makes the LUN look empty, which is sufficient.
  g_usb_msc.mediaPresent(false);
  g_usb_msc.end();
#endif

  // Tear down synchronously so the lock is always released, even if the
  // user navigates away before the visual progress finishes. Use
  // force_remount() because plain begin() has a 5-second rate limiter and
  // would no-op for a quick enable/disable cycle, leaving SDFS unmounted
  // and totalBytes()/numSectors() returning 0.
  SDInterface::i().unlock_for_msc();
  SDInterface::i().force_remount();
  updateSDCardInfo();

  _transfer_state = e_disabling_usb;
  _last_state_change = millis();
  _last_progress_update = millis();
  _transfer_progress = 0;
  strncpy(_status_message, "Refreshing SD card...", sizeof(_status_message));
  return true;
}

void USBTransferScreen::updateUSBState() {
  if (_transfer_state == e_enabling_usb) {
    // No real work to do here — TinyUSB enumeration happens asynchronously
    // and there's no public "mounted on host" signal we can poll without
    // hooking USB events. Run a short progress animation, then settle into
    // active state.
    uint32_t elapsed = millis() - _last_state_change;
    if (elapsed >= 1500) {
      _transfer_progress = 100;
      _transfer_state = e_usb_active;
      _usb_available = true;
      _usb_connected = true;
      strncpy(_status_message, "USB storage enabled", sizeof(_status_message));
      USB_LOG("USB MSC active - host should now see bGeigieZen Storage");
    } else {
      _transfer_progress = (elapsed * 100) / 1500;
    }
  } else if (_transfer_state == e_disabling_usb) {
    // SD has already been unmounted/remounted synchronously in
    // disableUSBStorage(); this branch only animates the progress bar
    // and settles back to idle.
    uint32_t elapsed = millis() - _last_state_change;
    if (elapsed >= 600) {
      _transfer_progress = 100;
      _transfer_state = e_idle;
      _usb_available = false;
      _usb_connected = false;
      strncpy(_status_message, "USB storage disabled", sizeof(_status_message));
      USB_LOG("USB MSC ejected; SD remounted, loggers will resume");
    } else {
      _transfer_progress = (elapsed * 100) / 600;
    }
  }
}

void USBTransferScreen::updateSDCardInfo() {
  if (SDInterface::i().ready()) {
    File root = SD.open("/");
    if (root) {
      _sd_file_count = countFilesRecursive(root);
      root.close();
    }
    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes = SD.usedBytes();
    _sd_total_space = totalBytes;
    _sd_free_space = totalBytes - usedBytes;
  } else {
    _sd_total_space = 0;
    _sd_free_space = 0;
    _sd_file_count = 0;
  }
}

uint32_t USBTransferScreen::countFilesRecursive(File dir) {
  uint32_t count = 0;
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    if (entry.isDirectory()) {
      count += countFilesRecursive(entry);
    } else {
      count++;
    }
    entry.close();
  }
  return count;
}

void USBTransferScreen::formatBytes(uint64_t bytes, char* buffer, size_t bufferSize) {
  if (bufferSize == 0) return;
  if (bytes < 1024) {
    snprintf(buffer, bufferSize, "%llu B", bytes);
  } else if (bytes < 1024ULL * 1024) {
    snprintf(buffer, bufferSize, "%.1f KB", bytes / 1024.0);
  } else if (bytes < 1024ULL * 1024 * 1024) {
    snprintf(buffer, bufferSize, "%.1f MB", bytes / (1024.0 * 1024.0));
  } else {
    snprintf(buffer, bufferSize, "%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
  }
}

uint32_t USBTransferScreen::countSDCardFiles() {
  return _sd_file_count;
}
