// beastvr.dll entry point. Loaded into the game by BeastVR Launcher.exe.
//
// Rule: do almost nothing inside DllMain (Windows holds the loader lock there).
// All real setup happens on our own thread.

#include <windows.h>

#include <MinHook.h>
#include <filesystem>
#include <string>

#include "hooks/present_hook.h"
#include "log.h"
#include "xr/xr_probe.h"

namespace {

HMODULE g_self = nullptr;

std::wstring ini_path() {
  wchar_t buf[MAX_PATH]{};
  GetModuleFileNameW(g_self, buf, MAX_PATH);
  return (std::filesystem::path(buf).parent_path() / L"beastvr.ini").wstring();
}

DWORD WINAPI init_thread(LPVOID) {
  beastvr::log::init();
  spdlog::info("BeastVR {} loaded into process {}", BEASTVR_VERSION, GetCurrentProcessId());

  const auto ini = ini_path();

  if (MH_Initialize() != MH_OK) {
    spdlog::critical("MinHook failed to initialize, stopping.");
    return 0;
  }

  if (!beastvr::hooks::install_present_hook()) {
    spdlog::error("Present hook failed, see lines above.");
  }

  if (GetPrivateProfileIntW(L"debug", L"openxr_probe", 1, ini.c_str()) != 0) {
    beastvr::xr::probe();
  }

  spdlog::info("Init finished.");
  return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    g_self = module;
    DisableThreadLibraryCalls(module);
    if (HANDLE t = CreateThread(nullptr, 0, init_thread, nullptr, 0, nullptr)) CloseHandle(t);
  }
  // DLL_PROCESS_DETACH: the game is exiting. Leave hooks alone; the process is
  // going away and touching them under the loader lock can deadlock.
  return TRUE;
}
