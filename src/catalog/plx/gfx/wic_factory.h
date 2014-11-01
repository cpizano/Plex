//#~def plx::CreateWICFactory
///////////////////////////////////////////////////////////////////////////////
// plx::CreateWICFactory : Windows Imaging components factory.
//
namespace plx {

plx::ComPtr<IWICImagingFactory> CreateWICFactory() {
  plx::ComPtr<IWICImagingFactory> factory;
  auto hr = ::CoCreateInstance(CLSID_WICImagingFactory, NULL,
                               CLSCTX_INPROC_SERVER,
                               __uuidof(factory),
                               reinterpret_cast<void **>(factory.GetAddressOf()));
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return factory;
}

}
