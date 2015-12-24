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
    waitable,
    port,
    pipe
  };

  Kernel32Exception(int line, Kind type)
      : Exception(line, "kernel 32"),
        error_code_(::GetLastError()),
        type_(type) {
    PostCtor();
  }

  Kind type() const {
    return type_;
  }

private:
  Kind type_;
  unsigned long error_code_;
};

}
