//#~def plx::WidthRectF
//#~def plx::HeightRectF
///////////////////////////////////////////////////////////////////////////////
//
//
namespace plx {

float WidthRectF(const D2D_RECT_F& r) {
  return r.right - r.left;
}

float HeightRectF(const D2D_RECT_F& r) {
  return r.bottom - r.top;
}

}
