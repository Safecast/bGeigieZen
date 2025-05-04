#include "file_browser.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/zen_button.h"
#include "utils/sd_wrapper.h"
#include <M5Unified.hpp>

FileBrowserScreen FileBrowserScreen_i;

FileBrowserScreen::FileBrowserScreen() : BaseScreen("File Browser", true),
                                       _fileCount(0),
                                       _selectedIndex(0),
                                       _currentPage(0),
                                       _totalPages(0),
                                       _viewingFileDetails(false) {
  required_sd = true;
  strcpy(_currentPath, "/");
  _selectedFilePath[0] = '\0';
  
  // Initialize marked for upload array
  for (int i = 0; i < FILE_BROWSER_MAX_FILES; i++) {
    _markedForUpload[i] = false;
  }
}

BaseScreen* FileBrowserScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (_viewingFileDetails) {
    // In file details view
    if (button1->is_fresh() && button1->get_data().shortPress) {
      // Back to file list
      _viewingFileDetails = false;
      force_next_render();
      return nullptr;
    }
    
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // Mark/unmark for upload
      for (int i = 0; i < _fileCount; i++) {
        if (strcmp(_files[i].path, _selectedFilePath) == 0) {
          _markedForUpload[i] = !_markedForUpload[i];
          force_next_render();
          break;
        }
      }
      return nullptr;
    }
  } else {
    // In file list view
    if (button1->is_fresh() && button1->get_data().shortPress) {
      // Navigate down in the list
      _selectedIndex++;
      if (_selectedIndex >= _fileCount) {
        _selectedIndex = 0;
      }
      
      // Adjust page if needed
      _currentPage = _selectedIndex / FILE_BROWSER_MAX_ITEMS_PER_PAGE;
      
      force_next_render();
      return nullptr;
    }
    
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // Select current item
      selectItem(_selectedIndex);
      force_next_render();
      return nullptr;
    }
  }
  
  if (button3->is_fresh() && button3->get_data().shortPress) {
    // Return to menu
    return &MenuWindow_i;
  }
  
  return nullptr;
}

void FileBrowserScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }

  clear_screen_content();
  
  if (_viewingFileDetails) {
    // Render file details view
    drawButton1("Back");
    drawButton2("Mark/Unmark");
    drawButton3("Menu");
    
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    
    // Find the file in our list
    FileItem* selectedFile = nullptr;
    bool isMarked = false;
    for (int i = 0; i < _fileCount; i++) {
      if (strcmp(_files[i].path, _selectedFilePath) == 0) {
        selectedFile = &_files[i];
        isMarked = _markedForUpload[i];
        break;
      }
    }
    
    if (selectedFile) {
      // Display file name
      M5.Lcd.setCursor(0, 40);
      M5.Lcd.setTextFont(4);
      M5.Lcd.printf("File: %s", selectedFile->name);
      
      // Display file path
      M5.Lcd.setCursor(0, 70);
      M5.Lcd.setTextFont(2);
      M5.Lcd.printf("Path: %s", selectedFile->path);
      
      // Display file size
      M5.Lcd.setCursor(0, 90);
      char sizeStr[20];
      formatSize(selectedFile->size, sizeStr, sizeof(sizeStr));
      M5.Lcd.printf("Size: %s", sizeStr);
      
      // Display file type
      M5.Lcd.setCursor(0, 110);
      M5.Lcd.printf("Type: %s", selectedFile->isDirectory ? "Directory" : "File");
      
      // Display marked status
      M5.Lcd.setCursor(0, 130);
      M5.Lcd.setTextColor(isMarked ? LCD_COLOR_ACTIVITY : LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("Status: %s", isMarked ? "Marked for upload" : "Not marked");
    } else {
      M5.Lcd.setCursor(0, 40);
      M5.Lcd.setTextFont(4);
      M5.Lcd.printf("File not found");
    }
  } else {
    // Render file list view
    drawButton1("Down");
    drawButton2("Select");
    drawButton3("Menu");
    
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.setTextFont(2);
    
    // Display current path
    M5.Lcd.printf("Path: %s\n", _currentPath);
    M5.Lcd.drawLine(0, 55, 320, 55, LCD_COLOR_DEFAULT);
    
    // Calculate page boundaries
    int startIdx = _currentPage * FILE_BROWSER_MAX_ITEMS_PER_PAGE;
    int endIdx = startIdx + FILE_BROWSER_MAX_ITEMS_PER_PAGE;
    if (endIdx > _fileCount) {
      endIdx = _fileCount;
    }
    
    // Display file list for current page
    for (int i = startIdx; i < endIdx; i++) {
      if (i == _selectedIndex) {
        M5.Lcd.setTextColor(LCD_COLOR_BACKGROUND, LCD_COLOR_DEFAULT);
      } else {
        M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      }
      
      // Display directory indicator and name
      if (_files[i].isDirectory) {
        M5.Lcd.printf("[%c] [DIR] %s\n", _markedForUpload[i] ? '*' : ' ', _files[i].name);
      } else {
        char sizeStr[20];
        formatSize(_files[i].size, sizeStr, sizeof(sizeStr));
        M5.Lcd.printf("[%c] %s (%s)\n", _markedForUpload[i] ? '*' : ' ', _files[i].name, sizeStr);
      }
    }
    
    // Display page indicator
    if (_totalPages > 1) {
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.setCursor(280, 220);
      M5.Lcd.printf("%d/%d", _currentPage + 1, _totalPages);
    }
  }
}

void FileBrowserScreen::navigateToDirectory(const char* path) {
  strcpy(_currentPath, path);
  _selectedIndex = 0;
  _currentPage = 0;
  refreshDirectoryListing();
}

void FileBrowserScreen::navigateUp() {
  // Find the last slash in the current path
  char* lastSlash = strrchr(_currentPath, '/');
  
  if (lastSlash == _currentPath) {
    // We're at the root directory
    strcpy(_currentPath, "/");
  } else {
    // Truncate the path at the last slash
    *lastSlash = '\0';
    
    // If the path is now empty, set it to root
    if (strlen(_currentPath) == 0) {
      strcpy(_currentPath, "/");
    }
  }
  
  _selectedIndex = 0;
  _currentPage = 0;
  refreshDirectoryListing();
}

void FileBrowserScreen::refreshDirectoryListing() {
  _fileCount = 0;
  
  // Clear all files
  for (int i = 0; i < FILE_BROWSER_MAX_FILES; i++) {
    _files[i].clear();
    _markedForUpload[i] = false;
  }
  
  // Check if SD card is available
  if (!SDInterface::i().ready()) {
    M5_LOGD("SD card not available");
    return;
  }
  
  // Ensure the path is valid
  if (strlen(_currentPath) == 0 || strlen(_currentPath) >= FILE_BROWSER_MAX_PATH_LENGTH) {
    strcpy(_currentPath, "/");
  }
  
  // Try to open the directory
  M5_LOGD("Opening directory: %s", _currentPath);
  File dir = SDInterface::i().get_file(_currentPath);
  
  // Check if directory opened successfully
  if (!dir) {
    M5_LOGD("Failed to open directory: %s", _currentPath);
    strcpy(_currentPath, "/");
    dir = SDInterface::i().get_file(_currentPath);
    if (!dir) {
      M5_LOGD("Failed to open root directory");
      return;
    }
  }
  
  // Check if it's actually a directory
  if (!dir.isDirectory()) {
    M5_LOGD("%s is not a directory", _currentPath);
    dir.close();
    strcpy(_currentPath, "/");
    dir = SDInterface::i().get_file(_currentPath);
    if (!dir || !dir.isDirectory()) {
      M5_LOGD("Failed to open root directory or not a directory");
      return;
    }
  }
  
  // Add parent directory entry if not at root
  if (strcmp(_currentPath, "/") != 0) {
    strcpy(_files[_fileCount].name, "..");
    strcpy(_files[_fileCount].path, "..");
    _files[_fileCount].isDirectory = true;
    _files[_fileCount].size = 0;
    _fileCount++;
  }
  
  // Read all files and directories
  File entry;
  while (_fileCount < FILE_BROWSER_MAX_FILES) {
    entry = dir.openNextFile();
    if (!entry) {
      break; // No more files
    }
    
    // Get the file name
    const char* entryName = entry.name();
    if (!entryName || strlen(entryName) == 0) {
      entry.close();
      continue;
    }
    
    // Skip hidden files
    if (entryName[0] == '.') {
      entry.close();
      continue;
    }
    
    // Ensure name isn't too long
    if (strlen(entryName) >= FILE_BROWSER_MAX_PATH_LENGTH - 1) {
      entry.close();
      continue;
    }
    
    // Store file information
    strcpy(_files[_fileCount].name, entryName);
    
    // Build the full path
    if (strcmp(_currentPath, "/") == 0) {
      snprintf(_files[_fileCount].path, FILE_BROWSER_MAX_PATH_LENGTH, "/%s", entryName);
    } else {
      snprintf(_files[_fileCount].path, FILE_BROWSER_MAX_PATH_LENGTH, "%s/%s", _currentPath, entryName);
    }
    
    _files[_fileCount].isDirectory = entry.isDirectory();
    _files[_fileCount].size = entry.size();
    
    _fileCount++;
    entry.close();
  }
  
  dir.close();
  M5_LOGD("Found %d files/directories", _fileCount);
  
  // Calculate total pages
  _totalPages = (_fileCount + FILE_BROWSER_MAX_ITEMS_PER_PAGE - 1) / FILE_BROWSER_MAX_ITEMS_PER_PAGE;
  if (_totalPages == 0) {
    _totalPages = 1;
  }
}

void FileBrowserScreen::selectItem(int index) {
  if (index < 0 || index >= _fileCount) {
    return;
  }
  
  if (_files[index].isDirectory) {
    // Navigate to directory
    if (strcmp(_files[index].name, "..") == 0) {
      navigateUp();
    } else {
      navigateToDirectory(_files[index].path);
    }
  } else {
    // View file details
    strcpy(_selectedFilePath, _files[index].path);
    _viewingFileDetails = true;
  }
}

void FileBrowserScreen::viewFileDetails(const char* path) {
  strcpy(_selectedFilePath, path);
  _viewingFileDetails = true;
}

void FileBrowserScreen::markFileForUpload(const char* path) {
  for (int i = 0; i < _fileCount; i++) {
    if (strcmp(_files[i].path, path) == 0) {
      _markedForUpload[i] = true;
      break;
    }
  }
}

const char* FileBrowserScreen::getFileExtension(const char* filename) {
  if (!filename) {
    return "";
  }
  const char* dot = strrchr(filename, '.');
  if (!dot || dot == filename) {
    return "";
  }
  return dot + 1;
}

void FileBrowserScreen::formatSize(uint32_t size, char* buffer, size_t bufferSize) {
  if (size < 1024) {
    snprintf(buffer, bufferSize, "%lu B", size);
  } else if (size < 1024 * 1024) {
    snprintf(buffer, bufferSize, "%.1f KB", size / 1024.0);
  } else if (size < 1024 * 1024 * 1024) {
    snprintf(buffer, bufferSize, "%.1f MB", size / (1024.0 * 1024.0));
  } else {
    snprintf(buffer, bufferSize, "%.1f GB", size / (1024.0 * 1024.0 * 1024.0));
  }
}

void FileBrowserScreen::enter_screen(Controller& controller) {
  M5_LOGD("Entering File Browser screen");
  
  // Initialize variables
  _viewingFileDetails = false;
  _selectedIndex = 0;
  _currentPage = 0;
  
  // Check if SD card is available
  if (!SDInterface::i().ready()) {
    M5_LOGD("SD card not available on enter_screen");
    // Use a custom status message
    set_status_message(F(" NO SD CARD INSERTED "));
  } else {
    // Refresh directory listing
    refreshDirectoryListing();
  }
  
  // Force render
  force_next_render();
}

void FileBrowserScreen::leave_screen(Controller& controller) {
  // Nothing to do when leaving the screen
}
