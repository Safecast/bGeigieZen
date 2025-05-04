#ifndef SCREENS_FILE_BROWSER_H
#define SCREENS_FILE_BROWSER_H

#include "base_screen.h"
#include "utils/sd_wrapper.h"

// Maximum number of items to display on one page
#define FILE_BROWSER_MAX_ITEMS_PER_PAGE 10
// Maximum path length for files and directories
#define FILE_BROWSER_MAX_PATH_LENGTH 255
// Maximum number of files to store in the list
#define FILE_BROWSER_MAX_FILES 50

class FileBrowserScreen : public BaseScreen {
 public:
  explicit FileBrowserScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  // File item structure to store file information
  struct FileItem {
    char name[FILE_BROWSER_MAX_PATH_LENGTH];
    char path[FILE_BROWSER_MAX_PATH_LENGTH];
    bool isDirectory;
    uint32_t size;
    
    void clear() {
      name[0] = '\0';
      path[0] = '\0';
      isDirectory = false;
      size = 0;
    }
  };

  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;
  
  // Navigate to a directory
  void navigateToDirectory(const char* path);
  
  // Navigate up one directory
  void navigateUp();
  
  // Refresh the current directory listing
  void refreshDirectoryListing();
  
  // Select a file or directory
  void selectItem(int index);
  
  // View file details
  void viewFileDetails(const char* path);
  
  // Mark a file for upload
  void markFileForUpload(const char* path);
  
  // Get file extension
  const char* getFileExtension(const char* filename);
  
  // Format size to human-readable string
  void formatSize(uint32_t size, char* buffer, size_t bufferSize);

 private:
  char _currentPath[FILE_BROWSER_MAX_PATH_LENGTH];
  FileItem _files[FILE_BROWSER_MAX_FILES];
  int _fileCount;
  int _selectedIndex;
  int _currentPage;
  int _totalPages;
  bool _viewingFileDetails;
  char _selectedFilePath[FILE_BROWSER_MAX_PATH_LENGTH];
  bool _markedForUpload[FILE_BROWSER_MAX_FILES];
};

extern FileBrowserScreen FileBrowserScreen_i;

#endif //SCREENS_FILE_BROWSER_H
