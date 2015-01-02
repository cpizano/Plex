//#~def plx::ScopedClipboard
///////////////////////////////////////////////////////////////////////////////
// plx::ScopedClipboard
//
namespace plx {

class ScopedClipboard {
  bool is_open_;

  ScopedClipboard(const ScopedClipboard&) = delete;
  ScopedClipboard& operator=(const ScopedClipboard&) = delete;
  void* operator new(size_t) = delete;

public:
  explicit ScopedClipboard(HWND window) {
    is_open_ = ::OpenClipboard(window) == TRUE;
  }

  bool did_open() const {
    return is_open_;
  }

  ~ScopedClipboard() {
    if (is_open_)
      ::CloseClipboard();
  }
};

}
