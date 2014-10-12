//#~def plx::MemoryException
///////////////////////////////////////////////////////////////////////////////
// plx::MemoryException
//
namespace plx {
class MemoryException : public plx::Exception {
  void* address_;

public:
  MemoryException(int line, void* address)
      : Exception(line, "Memory"), address_(address) {
    PostCtor();
  }
  void* address() const { return address_; }
};
}
