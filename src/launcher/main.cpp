// BeastVR Launcher: waits for the game process to appear, then loads
// beastvr.dll into it. Writes nothing into the game folder.
//
// Usage: run this, then start the game from Steam as normal.

#include <windows.h>
#include <tlhelp32.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

fs::path exe_dir() {
  wchar_t buf[MAX_PATH]{};
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  return fs::path(buf).parent_path();
}

std::wstring read_ini(const fs::path& ini, const wchar_t* section, const wchar_t* key) {
  wchar_t buf[512]{};
  GetPrivateProfileStringW(section, key, L"", buf, 512, ini.c_str());
  return buf;
}

DWORD find_process(const std::wstring& name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE) return 0;

  PROCESSENTRY32W pe{};
  pe.dwSize = sizeof(pe);
  DWORD pid = 0;
  for (BOOL more = Process32FirstW(snap, &pe); more; more = Process32NextW(snap, &pe)) {
    if (_wcsicmp(pe.szExeFile, name.c_str()) == 0) {
      pid = pe.th32ProcessID;
      break;
    }
  }
  CloseHandle(snap);
  return pid;
}

bool is_loaded(DWORD pid, const std::wstring& module_name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
  if (snap == INVALID_HANDLE_VALUE) return false;
  MODULEENTRY32W me{};
  me.dwSize = sizeof(me);
  bool found = false;
  for (BOOL more = Module32FirstW(snap, &me); more; more = Module32NextW(snap, &me)) {
    if (_wcsicmp(me.szModule, module_name.c_str()) == 0) {
      found = true;
      break;
    }
  }
  CloseHandle(snap);
  return found;
}

// Classic LoadLibraryW injection: write the DLL path into the game's memory and
// start a thread there that calls LoadLibraryW on it.
bool inject(DWORD pid, const fs::path& dll) {
  HANDLE proc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
                                PROCESS_VM_WRITE | PROCESS_VM_READ,
                            FALSE, pid);
  if (!proc) {
    std::wcerr << L"OpenProcess failed (error " << GetLastError() << L").\n";
    return false;
  }

  const std::wstring path = dll.wstring();
  const SIZE_T bytes = (path.size() + 1) * sizeof(wchar_t);
  void* remote = VirtualAllocEx(proc, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  bool ok = remote && WriteProcessMemory(proc, remote, path.c_str(), bytes, nullptr);

  if (ok) {
    // kernel32 sits at the same address in every process this boot, so our
    // LoadLibraryW address is valid inside the game too.
    void* proc_addr = reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
    auto load_library = reinterpret_cast<LPTHREAD_START_ROUTINE>(proc_addr);
    HANDLE thread = CreateRemoteThread(proc, nullptr, 0, load_library, remote, 0, nullptr);
    if (thread) {
      WaitForSingleObject(thread, 15000);
      CloseHandle(thread);
    } else {
      std::wcerr << L"CreateRemoteThread failed (error " << GetLastError() << L").\n";
      ok = false;
    }
  }

  if (remote) VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
  CloseHandle(proc);
  return ok && is_loaded(pid, dll.filename().wstring());
}

int finish(int code) {
  std::wcout << L"\nPress Enter to close.";
  std::wcin.get();
  return code;
}

}  // namespace

int wmain() {
  std::wcout << L"BeastVR Launcher " << BEASTVR_VERSION << L"\n\n";

  const fs::path dir = exe_dir();
  const fs::path ini = dir / L"beastvr.ini";
  const fs::path dll = dir / L"beastvr.dll";

  if (!fs::exists(dll)) {
    std::wcerr << L"beastvr.dll is missing next to the launcher.\n";
    return finish(1);
  }

  const std::wstring game = read_ini(ini, L"game", L"game_process");
  if (game.empty() || game == L"CHANGE_ME.exe") {
    std::wcerr << L"Set game_process in " << ini.wstring() << L"\n"
               << L"(the exe name from Task Manager > Details while the game runs).\n";
    return finish(1);
  }
  const DWORD delay_ms = GetPrivateProfileIntW(L"game", L"inject_delay_ms", 3000, ini.c_str());

  DWORD pid = find_process(game);
  if (pid) {
    std::wcout << game << L" is already running. Close it, then run the launcher before starting the game.\n";
    return finish(1);
  }

  std::wcout << L"Waiting for " << game << L"... start the game from Steam now.\n";
  while (!(pid = find_process(game))) Sleep(250);

  std::wcout << L"Found it (pid " << pid << L"). Waiting " << delay_ms << L" ms for it to settle...\n";
  Sleep(delay_ms);

  // Steam sometimes restarts the exe once on launch. Re-check.
  if (const DWORD now = find_process(game); now != pid) {
    if (!now) {
      std::wcerr << L"The game closed before BeastVR could load.\n";
      return finish(1);
    }
    std::wcout << L"Game restarted (new pid " << now << L"), using that one.\n";
    pid = now;
    Sleep(delay_ms);
  }

  if (!inject(pid, dll)) {
    std::wcerr << L"Loading beastvr.dll failed. Check docs/RECON.md for anti-cheat notes.\n";
    return finish(1);
  }

  std::wcout << L"BeastVR loaded. Log: %LOCALAPPDATA%\\BeastVR\\logs\\beastvr.log\n";
  return finish(0);
}
