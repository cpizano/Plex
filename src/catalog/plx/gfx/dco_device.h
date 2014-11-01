//#~def plx::CreateDCoDevice2
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoDevice2 : DirectComposition Desktop Device.
//
namespace plx {

plx::ComPtr<IDCompositionDesktopDevice> CreateDCoDevice2(
    plx::ComPtr<ID2D1Device> device2D) {
  plx::ComPtr<IDCompositionDesktopDevice> device;
  auto hr = DCompositionCreateDevice2(device2D.Get(),
                                      __uuidof(device),
                                      reinterpret_cast<void **>(device.GetAddressOf()));
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return device;
}

}
