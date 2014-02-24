// test_007, part of the plex test suite.

#include <windows.h>

///////////////////////////////////////////////////////////////////////////////
// plx::Exception
// line_ : The line of code, usually __LINE__.
// message_ : Whatever useful text.
namespace plx {
class Exception {
  int line_;
  const char* message_;

protected:
  void PostCtor() {
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

///////////////////////////////////////////////////////////////////////////////
// plx::IOException
// error_code_ : The win32 error code of the last operation.
namespace plx {
class IOException : public plx::Exception {
  DWORD error_code_;

public:
  IOException(int line)
      : Exception(line, "IO problem"), error_code_(::GetLastError()) {
    PostCtor();
  }
  DWORD ErrorCode() const { return error_code_; }
};
}

int wmain(int argc, wchar_t* argv[]) {
  if (argc > 3)
    throw plx::IOException(__LINE__);

  return 0;
}
