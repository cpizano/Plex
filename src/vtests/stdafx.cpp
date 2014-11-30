// This is the plex precompiled header cc.
// Do not edit this file by hand.

#include "stdafx.h"



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
plx::ComPtr<IDCompositionVisual2> CreateDCoVisual(plx::ComPtr<IDCompositionDesktopDevice> device) {
  plx::ComPtr<IDCompositionVisual2> visual;
  auto hr = device->CreateVisual(visual.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return visual;
}
plx::ComPtr<IDCompositionTarget> CreateDCoWindowTarget(
    plx::ComPtr<IDCompositionDesktopDevice> device, HWND window) {
  plx::ComPtr<IDCompositionTarget> target;
  auto hr = device->CreateTargetForHwnd(window, TRUE, target.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return target;
}
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
plx::ComPtr<IWICFormatConverter> CreateWICBitmapBGRA(unsigned int frame_index,
                                                     WICBitmapDitherType dither,
                                                     plx::ComPtr<IWICBitmapDecoder> decoder,
                                                     plx::ComPtr<IWICImagingFactory> factory) {
  plx::ComPtr<IWICFormatConverter> converter;
  auto hr = factory->CreateFormatConverter(converter.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  plx::ComPtr<IWICBitmapFrameDecode> frame;
  hr = decoder->GetFrame(frame_index, frame.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  hr = converter->Initialize(frame.Get(),
                             GUID_WICPixelFormat32bppPBGRA,
                             dither,
                             nullptr,
                             0.0f,
                             WICBitmapPaletteTypeCustom);
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return converter;
}
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
plx::ComPtr<ID2D1DeviceContext> CreateDCoDeviceCtx(
    plx::ComPtr<IDCompositionSurface> surface,
    const plx::DPI& dpi, const D2D1_SIZE_F& extra_offset) {
  plx::ComPtr<ID2D1DeviceContext> dc;
  POINT offset;
  auto hr = surface->BeginDraw(nullptr,
                               __uuidof(dc),
                               reinterpret_cast<void **>(dc.GetAddressOf()),
                               &offset);
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  dc->SetDpi(float(dpi.get_dpi_x()), float(dpi.get_dpi_y()));
  auto matrix = D2D1::Matrix3x2F::Translation(
      dpi.to_logical_x(offset.x) + extra_offset.width,
      dpi.to_logical_y(offset.y) + extra_offset.height);
  dc->SetTransform(matrix);
  return dc;
}
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
plx::ComPtr<IWICBitmapDecoder> CreateWICDecoder(
    plx::ComPtr<IWICImagingFactory> factory, const plx::FilePath& fname) {
  plx::ComPtr<IWICBitmapDecoder> decoder;
  auto hr = factory->CreateDecoderFromFilename(fname.raw(), nullptr,
                                               GENERIC_READ,
                                               WICDecodeMetadataCacheOnDemand,
                                               decoder.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return decoder;
}
float WidthRectF(const D2D_RECT_F& r) {
  return r.right - r.left;
}
float HeightRectF(const D2D_RECT_F& r) {
  return r.bottom - r.top;
}
}
