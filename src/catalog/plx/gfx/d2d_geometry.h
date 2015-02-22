//#~def plx::CreateD2D1Geometry
///////////////////////////////////////////////////////////////////////////////
// Makes Direct2D's ID2D1Geometry objects.
//
namespace plx {

plx::ComPtr<ID2D1Geometry> CreateD2D1Geometry(
    plx::ComPtr<ID2D1Factory2> d2d1_factory,
    const D2D1_ELLIPSE& ellipse) {
  plx::ComPtr<ID2D1EllipseGeometry> geometry;
  auto hr = d2d1_factory->CreateEllipseGeometry(ellipse, geometry.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return geometry;
}

plx::ComPtr<ID2D1Geometry> CreateD2D1Geometry(
    plx::ComPtr<ID2D1Factory2> d2d1_factory,
    const D2D1_ROUNDED_RECT& rect) {
  plx::ComPtr<ID2D1RoundedRectangleGeometry> geometry;
  auto hr = d2d1_factory->CreateRoundedRectangleGeometry(rect, geometry.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return geometry;
}

}
