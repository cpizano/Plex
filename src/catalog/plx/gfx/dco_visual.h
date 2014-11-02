//#~def plx::CreateDCoVisual
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoVisual : Makes a DirectComposition Visual.
//
namespace plx {

plx::ComPtr<IDCompositionVisual2> CreateDCoVisual(plx::ComPtr<IDCompositionDesktopDevice> device) {
  plx::ComPtr<IDCompositionVisual2> visual;
  auto hr = device->CreateVisual(visual.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return visual;
}

}
