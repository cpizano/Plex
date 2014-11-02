//#~def plx::CreateDCoWindowTarget
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoWindowTarget : Makes a DirectComposition target for a window.
//
namespace plx {

plx::ComPtr<IDCompositionTarget> CreateDCoWindowTarget(
    plx::ComPtr<IDCompositionDesktopDevice> device, HWND window) {
  plx::ComPtr<IDCompositionTarget> target;
  auto hr = device->CreateTargetForHwnd(window, TRUE, target.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return target;
}

}
