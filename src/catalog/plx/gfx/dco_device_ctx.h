//#~def plx::CreateDCoDeviceCtx
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoDeviceCtx : Makes a DirectComposition DC for drawing.
//
namespace plx {

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

}
