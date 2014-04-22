//#~def plx::RangeException
///////////////////////////////////////////////////////////////////////////////
// plx::RangeException
//
namespace plx {
class RangeException : public plx::Exception {
  void* ptr_;
public:
  RangeException(int line, void* ptr)
      : Exception(line, "Invalid Range"), ptr_(ptr) {
    PostCtor();
  }

  void* pointer() const {
    return ptr_;
  }
};
}
