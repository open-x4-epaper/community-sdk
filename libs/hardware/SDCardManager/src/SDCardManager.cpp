#include "SDCardManager.h"

#include <FsCompat.h>

namespace {
// constexpr uint8_t SD_CS = 12; // Unused in SD_MMC
// constexpr uint32_t SPI_FQ = 40000000; // Unused
}

SDCardManager SDCardManager::instance;

SDCardManager::SDCardManager() {}  // Removing sd() init

bool SDCardManager::begin() {
  // CLK 39, CMD 40, D0 38, D1 48, D2 42, D3 41
  SD_MMC.setPins(39, 40, 38, 48, 42, 41);
  if (!SD_MMC.begin("/sdcard",
                    false)) {  // false = 4-bit mode capability check (but actually second
                               // arg is mode1bit. false means try 4bit)
    if (Serial) Serial.printf("[%lu] [SD] SD card not detected or init failed\n", millis());
    initialized = false;
  } else {
    if (Serial) Serial.printf("[%lu] [SD] SD card detected\n", millis());
    initialized = true;
  }

  return initialized;
}

bool SDCardManager::ready() const { return initialized; }

std::vector<String> SDCardManager::listFiles(const char* path, const int maxFiles) {
  std::vector<String> ret;
  if (!initialized) {
    if (Serial) Serial.printf("[%lu] [SD] not initialized, returning empty list\n", millis());
    return ret;
  }

  File root = SD_MMC.open(path);
  if (!root) {
    if (Serial) Serial.printf("[%lu] [SD] Failed to open directory\n", millis());
    return ret;
  }
  if (!root.isDirectory()) {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    root.close();
    return ret;
  }

  int count = 0;
  File f = root.openNextFile();
  while (f && count < maxFiles) {
    if (f.isDirectory()) {
      f = root.openNextFile();
      continue;
    }
    const char* n = f.name();
    // Strip leading path if present to match SdFat behavior of getName
    const char* name = strrchr(n, '/');
    ret.emplace_back(name ? name + 1 : n);

    f = root.openNextFile();
    count++;
  }
  root.close();
  return ret;
}

String SDCardManager::readFile(const char* path) {
  if (!initialized) {
    if (Serial) Serial.printf("[%lu] [SD] not initialized; cannot read file\n", millis());
    return {""};
  }

  FsFile f;
  if (!openFileForRead("SD", path, f)) {
    return {""};
  }

  String content = "";
  constexpr size_t maxSize = 50000;  // Limit to 50KB
  size_t readSize = 0;
  while (f.available() && readSize < maxSize) {
    const char c = static_cast<char>(f.read());
    content += c;
    readSize++;
  }
  f.close();
  return content;
}

bool SDCardManager::readFileToStream(const char* path, Print& out, const size_t chunkSize) {
  if (!initialized) {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    if (Serial) Serial.println("SDCardManager: not initialized; cannot read file");
    return false;
  }

  FsFile f;
  if (!openFileForRead("SD", path, f)) {
    return false;
  }

  constexpr size_t localBufSize = 256;
  uint8_t buf[localBufSize];
  const size_t toRead = (chunkSize == 0) ? localBufSize : (chunkSize < localBufSize ? chunkSize : localBufSize);

  while (f.available()) {
    const int r = f.read(buf, toRead);
    if (r > 0) {
      out.write(buf, static_cast<size_t>(r));
    } else {
      break;
    }
  }

  f.close();
  return true;
}

size_t SDCardManager::readFileToBuffer(const char* path, char* buffer, const size_t bufferSize, const size_t maxBytes) {
  if (!buffer || bufferSize == 0) return 0;
  if (!initialized) {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    if (Serial) Serial.println("SDCardManager: not initialized; cannot read file");
    buffer[0] = '\0';
    return 0;
  }

  FsFile f;
  if (!openFileForRead("SD", path, f)) {
    buffer[0] = '\0';
    return 0;
  }

  const size_t maxToRead = (maxBytes == 0) ? (bufferSize - 1) : min(maxBytes, bufferSize - 1);
  size_t total = 0;

  while (f.available() && total < maxToRead) {
    constexpr size_t chunk = 64;
    const size_t want = maxToRead - total;
    const size_t readLen = (want < chunk) ? want : chunk;
    const int r = f.read(reinterpret_cast<uint8_t*>(buffer) + total, readLen);
    if (r > 0) {
      total += static_cast<size_t>(r);
    } else {
      break;
    }
  }

  buffer[total] = '\0';
  f.close();
  return total;
}

bool SDCardManager::writeFile(const char* path, const String& content) {
  if (!initialized) {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    if (Serial) Serial.println("SDCardManager: not initialized; cannot write file");
    return false;
  }

  // NOTE: SD_MMC / FS open with "w" (FILE_WRITE) truncates by default, so
  // explicit remove is not strictly needed but good for safety.
  if (SD_MMC.exists(path)) {
    SD_MMC.remove(path);
  }

  FsFile f;
  if (!openFileForWrite("SD", path, f)) {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    if (Serial) Serial.printf("Failed to open file for write: %s\n", path);
    return false;
  }

  const size_t written = f.print(content);
  f.close();
  return written == content.length();
}

bool SDCardManager::ensureDirectoryExists(const char* path) {
  if (!initialized) {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    if (Serial) Serial.println("SDCardManager: not initialized; cannot create directory");
    return false;
  }

  // Check if directory already exists
  if (SD_MMC.exists(path)) {
    File dir = SD_MMC.open(path);
    if (dir && dir.isDirectory()) {
      dir.close();
      if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
      if (Serial) Serial.printf("Directory already exists: %s\n", path);
      return true;
    }
    dir.close();
  }

  // Create the directory
  if (SD_MMC.mkdir(path)) {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    if (Serial) Serial.printf("Created directory: %s\n", path);
    return true;
  } else {
    if (Serial) Serial.printf("[%lu] [SD] Path is not a directory\n", millis());
    if (Serial) Serial.printf("Failed to create directory: %s\n", path);
    return false;
  }
}

bool SDCardManager::openFileForRead(const char* moduleName, const char* path, FsFile& file) {
  if (!SD_MMC.exists(path)) {
    if (Serial) Serial.printf("[%lu] [%s] File does not exist: %s\n", millis(), moduleName, path);
    return false;
  }

  file = FsFile(SD_MMC.open(path, FILE_READ));
  if (!file) {
    if (Serial) Serial.printf("[%lu] [%s] Failed to open file for reading: %s\n", millis(), moduleName, path);
    return false;
  }
  return true;
}

bool SDCardManager::openFileForRead(const char* moduleName, const std::string& path, FsFile& file) {
  return openFileForRead(moduleName, path.c_str(), file);
}

bool SDCardManager::openFileForRead(const char* moduleName, const String& path, FsFile& file) {
  return openFileForRead(moduleName, path.c_str(), file);
}

bool SDCardManager::openFileForWrite(const char* moduleName, const char* path, FsFile& file) {
  file = FsFile(SD_MMC.open(path, FILE_WRITE));
  if (!file) {
    if (Serial) Serial.printf("[%lu] [%s] Failed to open file for writing: %s\n", millis(), moduleName, path);
    return false;
  }
  return true;
}

bool SDCardManager::openFileForWrite(const char* moduleName, const std::string& path, FsFile& file) {
  return openFileForWrite(moduleName, path.c_str(), file);
}

bool SDCardManager::openFileForWrite(const char* moduleName, const String& path, FsFile& file) {
  return openFileForWrite(moduleName, path.c_str(), file);
}

bool SDCardManager::removeDir(const char* path) {
  // 1. Open the directory
  File dir = SD_MMC.open(path);
  if (!dir) {
    return false;
  }
  if (!dir.isDirectory()) {
    return false;
  }

  File file = dir.openNextFile();
  char name[128];
  while (file) {
    String filePath = path;
    if (!filePath.endsWith("/")) {
      filePath += "/";
    }
    const char* n = file.name();
    const char* leafName = strrchr(n, '/');
    filePath += (leafName ? leafName + 1 : n);

    // Check if it's a directory? file.isDirectory().
    // Recursion needed.
    if (file.isDirectory()) {
      file.close();  // Close before recursing usually good
      if (!removeDir(filePath.c_str())) {
        return false;
      }
    } else {
      file.close();
      if (!SD_MMC.remove(filePath.c_str())) {
        return false;
      }
    }
    file = dir.openNextFile();
  }

  // Close dir before removing?
  dir.close();
  return SD_MMC.rmdir(path);
}
