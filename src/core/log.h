#pragma once

#include <spdlog/spdlog.h>

namespace beastvr::log {

// Logs to %LOCALAPPDATA%\BeastVR\logs\beastvr.log (overwritten each session).
void init();
void shutdown();

}  // namespace beastvr::log
