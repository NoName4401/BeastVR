#include "hooks/present_hook.h"

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include <MinHook.h>
#include <atomic>
#include <cstdint>
#include <spdlog/spdlog.h>

namespace beastvr::hooks {
namespace {

using PresentFn = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);

// IDXGISwapChain vtable: IUnknown (0-2), IDXGIObject (3-6),
// IDXGIDeviceSubObject (7), then Present is slot 8.
constexpr int kPresentSlot = 8;

PresentFn g_original = nullptr;
void* g_target = nullptr;
std::atomic<std::uint64_t> g_frames{0};

void log_first_frame(IDXGISwapChain* swapchain) {
  DXGI_SWAP_CHAIN_DESC desc{};
  if (SUCCEEDED(swapchain->GetDesc(&desc))) {
    spdlog::info("Present hook live: {}x{}, format {}, {} buffers", desc.BufferDesc.Width,
                 desc.BufferDesc.Height, static_cast<int>(desc.BufferDesc.Format), desc.BufferCount);
  }

  ID3D11Device* device = nullptr;
  if (SUCCEEDED(swapchain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device)))) {
    spdlog::info("Game renders with D3D11, feature level 0x{:x}",
                 static_cast<unsigned>(device->GetFeatureLevel()));
    device->Release();
  } else {
    spdlog::warn("Game swapchain is not D3D11. Set the game to DirectX 11 for now (Phase 0 targets DX11).");
  }
}

HRESULT STDMETHODCALLTYPE hooked_present(IDXGISwapChain* swapchain, UINT sync_interval, UINT flags) {
  const auto frame = ++g_frames;
  if (frame == 1) {
    log_first_frame(swapchain);
  } else if (frame % 1000 == 0) {
    spdlog::info("Frames presented: {}", frame);
  }

  // Phase 2+: copy eye textures and submit to OpenXR here.
  return g_original(swapchain, sync_interval, flags);
}

// Builds a throwaway D3D11 device + swapchain on a hidden window just to read
// the address of Present out of its vtable. The game's swapchain shares the
// same implementation in dxgi.dll, so hooking that address catches the game.
void* find_present_address() {
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = DefWindowProcW;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"BeastVRDummyWindow";
  RegisterClassExW(&wc);

  HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, nullptr,
                              nullptr, wc.hInstance, nullptr);
  if (!hwnd) {
    spdlog::error("Dummy window failed (error {})", GetLastError());
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return nullptr;
  }

  DXGI_SWAP_CHAIN_DESC sd{};
  sd.BufferCount = 1;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  IDXGISwapChain* swapchain = nullptr;
  ID3D11Device* device = nullptr;
  ID3D11DeviceContext* context = nullptr;
  const HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                                   D3D11_SDK_VERSION, &sd, &swapchain, &device, nullptr, &context);

  void* present = nullptr;
  if (SUCCEEDED(hr) && swapchain) {
    void** vtable = *reinterpret_cast<void***>(swapchain);
    present = vtable[kPresentSlot];
  } else {
    spdlog::error("Dummy D3D11 swapchain failed (hr 0x{:08x})", static_cast<unsigned>(hr));
  }

  if (swapchain) swapchain->Release();
  if (context) context->Release();
  if (device) device->Release();
  DestroyWindow(hwnd);
  UnregisterClassW(wc.lpszClassName, wc.hInstance);
  return present;
}

}  // namespace

bool install_present_hook() {
  g_target = find_present_address();
  if (!g_target) return false;

  if (MH_CreateHook(g_target, reinterpret_cast<void*>(&hooked_present), reinterpret_cast<void**>(&g_original)) !=
      MH_OK) {
    spdlog::error("MH_CreateHook(Present) failed");
    return false;
  }
  if (MH_EnableHook(g_target) != MH_OK) {
    spdlog::error("MH_EnableHook(Present) failed");
    return false;
  }
  spdlog::info("Present hooked at {}", g_target);
  return true;
}

void remove_present_hook() {
  if (!g_target) return;
  MH_DisableHook(g_target);
  MH_RemoveHook(g_target);
  g_target = nullptr;
}

}  // namespace beastvr::hooks
