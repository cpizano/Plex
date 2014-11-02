//#~def plx::CreateDCoSurface
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoSurface : Makes a DirectComposition compatible surface.
//
namespace plx {

plx::ComPtr<IDCompositionSurface> CreateDCoSurface(
    plx::ComPtr<IDCompositionDesktopDevice> device, unsigned int w, unsigned int h) {
  plx::ComPtr<IDCompositionSurface> surface;
  auto hr = device->CreateSurface(w, h,
                                  DXGI_FORMAT_B8G8R8A8_UNORM,
                                  DXGI_ALPHA_MODE_PREMULTIPLIED,
                                  surface.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return surface;
}

}
