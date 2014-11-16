//#~def plx::RectL
///////////////////////////////////////////////////////////////////////////////
// plx::RectL : windows compatible RECT wrapper.
//
namespace plx {

struct RectL : public ::RECT {
  RectL() : RECT() {}
  RectL(long x0, long y0, long x1, long y1) : RECT({x0, y0, x1, y1}) {}
  RectL(const plx::SizeL& size): RECT({ 0L, 0L, size.cx, size.cy }) {}

  plx::SizeL size() { return SizeL(right - left, bottom - top); }
  long width() const { return right - left; }
  long height() const { return bottom - top; }
};

}
