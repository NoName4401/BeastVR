#pragma once

namespace beastvr::xr {

// Phase 0 check: is there an OpenXR runtime, does it support D3D11, and is a
// headset connected? Logs the answers, then tears everything down again.
bool probe();

}  // namespace beastvr::xr
