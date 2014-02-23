//#~def plx::Exception
///////////////////////////////////////////////////////////////////////////////
// plx::Exception
// line_ : The line of code, usually __LINE__.
// message_ : Whatever useful text.
namespace plx {
class Exception {         //#~base class
  int line_;              //#~warn negative
  const char* message_;   //#~rom pointer | exposed | zt-ascii

protected:
  void PostCtor() {       //#~should derived
    if (::IsDebuggerPresent()) {
      __debugbreak();
    }
  }

public:
  Exception(int line, const char* message) : line_(line), message_(message) {}
  virtual ~Exception() {}
  const char* Message() const { return message_; }
  int Line() const { return line_; }
};
}
