//#~def plx::Kernel32Exception
///////////////////////////////////////////////////////////////////////////////
// plx::Kernel32Exception (thrown by kernel32 functions)
//
namespace plx {
class Kernel32Exception : public plx::Exception {
public:
  enum Kind {
    memory,
    thread,
    process,
    waitable
  };

  Kernel32Exception(int line, Kind type)
      : Exception(line, "kernel 32"), type_(type) {
    PostCtor();
  }

  Kind type() const {
    return type_;
  }

private:
  Kind type_;
};
}
