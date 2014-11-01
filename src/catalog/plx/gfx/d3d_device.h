//#~def plx::CreateDeviceD3D11
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDeviceD3D11 : Direct3D device fatory.
//
namespace plx {

plx::ComPtr<ID3D11Device> CreateDeviceD3D11(int extra_flags) {
  auto flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
               D3D11_CREATE_DEVICE_SINGLETHREADED;
  flags |= extra_flags;

  plx::ComPtr<ID3D11Device> device;
  auto hr = D3D11CreateDevice(nullptr,
                              D3D_DRIVER_TYPE_HARDWARE,
                              nullptr,
                              flags,
                              nullptr, 0,
                              D3D11_SDK_VERSION,
                              device.GetAddressOf(),
                              nullptr,
                              nullptr);
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return device;
}

}
