#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>

#include "absl/strings/str_format.h"
#include "test/server.h"

uint64_t GetFileSize(std::string_view path) {
  std::error_code error;
  uint64_t size = std::filesystem::file_size(path, error);
  if (error) {
    return 0;
  }
  return size;
}

int DelayedSum(int value) {
  std::this_thread::sleep_for(std::chrono::seconds(5));
  return value + 9999;
}

int ReadEntireFile(std::string_view path) {
  static constexpr const int kBufferSize = 65535;

  std::ifstream file((std::string(path)));
  if (!file.is_open()) {
    return -1;
  }

  absl::PrintF("Reading file %s...\n", path);

  std::string output;
  output.reserve(GetFileSize(path));

  auto buffer = std::unique_ptr<char[]>(new char[kBufferSize]);
  std::string::size_type size = 0;
  while (!file.eof()) {
    file.read(buffer.get(), kBufferSize);
    std::streamsize bytes = file.gcount();

    output.append(buffer.get(), bytes);

    size += bytes;
  }

  absl::PrintF("Read %d bytes from file %s.\n", output.size(), path);
  return size;
}

int ReadPacketFromSession(Session* session) {
  std::string data;
  printf("Read %lu bytes\n", session->Read(data));
  return 0;
}
