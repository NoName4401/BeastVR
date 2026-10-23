#include "log.h"

#include <windows.h>
#include <shlobj.h>

#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>

namespace beastvr::log {
namespace {

std::filesystem::path log_dir() {
  PWSTR raw = nullptr;
  std::filesystem::path dir = std::filesystem::temp_directory_path();
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &raw))) {
    dir = raw;
  }
  CoTaskMemFree(raw);
  return dir / L"BeastVR" / L"logs";
}

}  // namespace

void init() {
  const auto dir = log_dir();
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);

  auto logger = spdlog::basic_logger_mt("beastvr", (dir / L"beastvr.log").string(), /*truncate=*/true);
  logger->set_pattern("[%H:%M:%S.%e] [%l] %v");
  logger->flush_on(spdlog::level::info);  // a crash must not eat the last lines
  spdlog::set_default_logger(logger);
}

void shutdown() { spdlog::shutdown(); }

}  // namespace beastvr::log
