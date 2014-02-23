//#~def plx::IOException
///////////////////////////////////////////////////////////////////////////////
// plx::IOException
// error_code_ : The win32 error code of the last operation.
namespace plx {
class IOException : public plx::Exception {
  DWORD error_code_;

public:
  IOException(int line)
      : PlexException(line, "IO problem"), error_code_(::GetLastError()) {
    PostCtor();
  }
  DWORD ErrorCode() const { return error_code_; }
};
}
