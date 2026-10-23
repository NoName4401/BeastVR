#pragma once

namespace beastvr::hooks {

// Hooks IDXGISwapChain::Present, which every D3D11 and D3D12 game calls once
// per frame. Phase 0 only logs from it. Later phases submit VR frames here.
// Call after MH_Initialize().
bool install_present_hook();
void remove_present_hook();

}  // namespace beastvr::hooks
