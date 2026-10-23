#include "xr/xr_probe.h"

#include <openxr/openxr.h>

#include <cstring>
#include <spdlog/spdlog.h>
#include <string_view>
#include <vector>

namespace beastvr::xr {
namespace {

constexpr const char* kD3D11Ext = "XR_KHR_D3D11_enable";
constexpr const char* kD3D12Ext = "XR_KHR_D3D12_enable";

}  // namespace

bool probe() {
  uint32_t count = 0;
  XrResult r = xrEnumerateInstanceExtensionProperties(nullptr, 0, &count, nullptr);
  if (XR_FAILED(r)) {
    spdlog::error("No OpenXR runtime found (result {}). Set Virtual Desktop, Quest Link or SteamVR as the active OpenXR runtime.",
                  static_cast<int>(r));
    return false;
  }

  XrExtensionProperties blank{XR_TYPE_EXTENSION_PROPERTIES};
  std::vector<XrExtensionProperties> exts(count, blank);
  xrEnumerateInstanceExtensionProperties(nullptr, count, &count, exts.data());

  bool has_d3d11 = false;
  bool has_d3d12 = false;
  for (const auto& e : exts) {
    const std::string_view name = e.extensionName;
    has_d3d11 |= name == kD3D11Ext;
    has_d3d12 |= name == kD3D12Ext;
  }
  spdlog::info("OpenXR runtime supports D3D11: {}, D3D12: {}", has_d3d11, has_d3d12);
  if (!has_d3d11) spdlog::warn("Runtime has no D3D11 support, which Phase 2 needs.");

  XrInstanceCreateInfo ci{XR_TYPE_INSTANCE_CREATE_INFO};
  std::strncpy(ci.applicationInfo.applicationName, "BeastVR", XR_MAX_APPLICATION_NAME_SIZE - 1);
  std::strncpy(ci.applicationInfo.engineName, "BeastVR", XR_MAX_ENGINE_NAME_SIZE - 1);
  ci.applicationInfo.applicationVersion = 1;
  ci.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);  // widest runtime support
  const char* enabled[] = {kD3D11Ext};
  ci.enabledExtensionCount = has_d3d11 ? 1 : 0;
  ci.enabledExtensionNames = enabled;

  XrInstance instance = XR_NULL_HANDLE;
  r = xrCreateInstance(&ci, &instance);
  if (XR_FAILED(r)) {
    spdlog::error("xrCreateInstance failed (result {})", static_cast<int>(r));
    return false;
  }

  XrInstanceProperties ip{XR_TYPE_INSTANCE_PROPERTIES};
  if (XR_SUCCEEDED(xrGetInstanceProperties(instance, &ip))) {
    spdlog::info("OpenXR runtime: {} {}.{}.{}", ip.runtimeName, XR_VERSION_MAJOR(ip.runtimeVersion),
                 XR_VERSION_MINOR(ip.runtimeVersion), XR_VERSION_PATCH(ip.runtimeVersion));
  }

  XrSystemGetInfo sgi{XR_TYPE_SYSTEM_GET_INFO};
  sgi.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
  XrSystemId system = XR_NULL_SYSTEM_ID;
  r = xrGetSystem(instance, &sgi, &system);

  bool ok = false;
  if (XR_FAILED(r)) {
    spdlog::warn("No headset found (result {}). Is it connected and awake?", static_cast<int>(r));
  } else {
    XrSystemProperties sp{XR_TYPE_SYSTEM_PROPERTIES};
    xrGetSystemProperties(instance, system, &sp);
    spdlog::info("OpenXR system: {} (orientation tracking {}, position tracking {})", sp.systemName,
                 sp.trackingProperties.orientationTracking == XR_TRUE,
                 sp.trackingProperties.positionTracking == XR_TRUE);

    uint32_t views = 0;
    xrEnumerateViewConfigurationViews(instance, system, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &views,
                                      nullptr);
    XrViewConfigurationView blank_view{XR_TYPE_VIEW_CONFIGURATION_VIEW};
    std::vector<XrViewConfigurationView> view_info(views, blank_view);
    xrEnumerateViewConfigurationViews(instance, system, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, views, &views,
                                      view_info.data());
    for (uint32_t i = 0; i < views; ++i) {
      spdlog::info("Eye {} recommended {}x{}", i, view_info[i].recommendedImageRectWidth,
                   view_info[i].recommendedImageRectHeight);
    }
    ok = true;
  }

  xrDestroyInstance(instance);
  return ok;
}

}  // namespace beastvr::xr
