//#~def plx::SizeL
///////////////////////////////////////////////////////////////////////////////
// plx::SizeL : windows compatible SIZE wrapper.
//
namespace plx {

struct SizeL : public ::SIZE {
  SizeL() : SIZE() {}
  SizeL(long width, long height) : SIZE({width, height}) {}
  bool empty() const { return (cx == 0L) && (cy == 0L); }
};

}
