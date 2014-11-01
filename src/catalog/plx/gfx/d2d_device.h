//#~def plx::CreateD2D1FactoryST
//#~def plx::CreateDeviceD2D1
///////////////////////////////////////////////////////////////////////////////
// plx::CreateD2D1FactoryST : Direct2D fatory.
// plx::CreateDeviceD2D1 : Direct2D device.
//
namespace plx {

plx::ComPtr<ID2D1Factory2> CreateD2D1FactoryST(D2D1_DEBUG_LEVEL debug_level) {
  D2D1_FACTORY_OPTIONS options = {};
  options.debugLevel = debug_level;

  plx::ComPtr<ID2D1Factory2> factory;
  auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                              options,
                              factory.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return factory;
}

plx::ComPtr<ID2D1Device> CreateDeviceD2D1(plx::ComPtr<ID3D11Device> device3D,
                                          plx::ComPtr<ID2D1Factory2> factoryD2D1) {
  plx::ComPtr<IDXGIDevice3> dxgi_dev;
  device3D.As(&dxgi_dev);
  plx::ComPtr<ID2D1Device> device2D;

  auto hr = factoryD2D1->CreateDevice(dxgi_dev.Get(),
                                      device2D.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return device2D;
}

}
