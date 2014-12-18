//#~def plx::CreateDWriteFactory
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDWriteFactory : DirectWrite shared factory.
//
namespace plx {

plx::ComPtr<IDWriteFactory> CreateDWriteFactory() {
  plx::ComPtr<IDWriteFactory> factory;
  auto hr = ::DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(IDWriteFactory),
      reinterpret_cast<IUnknown**>(factory.GetAddressOf()));
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return factory;
}

}
