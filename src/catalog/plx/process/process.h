//#~def plx::Process
///////////////////////////////////////////////////////////////////////////////
// plx::Process
//
namespace plx {

class Process {
  HANDLE handle_;
  HANDLE thread_;
  unsigned long error_;

private:
  Process(HANDLE handle, HANDLE thread, unsigned long error)
    : handle_(handle), thread_(thread), error_(error) {
  }

  Process(const Process&) = delete;
  Process& operator=(const Process&) = delete;

public:
  Process() : handle_(0UL), thread_(0UL), error_(NO_ERROR) {}

  Process(Process&& process)
    : handle_(0), thread_(0), error_(NO_ERROR) {
    std::swap(process.handle_, handle_);
    std::swap(process.thread_, thread_);
    std::swap(process.error_, error_);
  }

  bool wait_termination(unsigned long millisecs) {
    if (WAIT_OBJECT_0 == ::WaitForSingleObject(handle_, millisecs)) {
      ::GetExitCodeProcess(handle_, &error_);
      return true;
    }
    return false;
  }

  bool kill(unsigned int ret_code) {
    if (!::TerminateProcess(handle_, ret_code))
      return false;
    Process closer(std::move(*this));
    return true;
  }

  unsigned long resume() {
    return ::ResumeThread(thread_);
  }

  ~Process() {
    if (handle_) {
      if (!::CloseHandle(handle_))
        __debugbreak();
      if (!::CloseHandle(thread_))
        __debugbreak();
    }
  }

  Process& operator=(Process&& process) {
    std::swap(process.handle_, handle_);
    std::swap(process.thread_, thread_);
  }

  bool is_valid() const { return (handle_ != 0UL) && (thread_ != 0UL); }
  unsigned long error() const { return error_; }

  static Process Create(const plx::FilePath& path,
    const std::wstring& args,
    const plx::ProcessParams& pp) {
    const wchar_t* fmt = (path.has_spaces()) ? L"\"%s\" %s" : L"%s %s";
    auto cmd_line = plx::StringPrintf(fmt, path.raw(), args.c_str());

    STARTUPINFO si = { sizeof(si), 0 };
    PROCESS_INFORMATION pi = {};

    auto flags = pp.job_ ? pp.flags_ | CREATE_SUSPENDED : pp.flags_;

    if (!::CreateProcessW(NULL, &cmd_line[0],
      nullptr, nullptr,
      pp.inherit_ ? TRUE : FALSE,
      flags,
      pp.env_, nullptr,
      &si, &pi)) {
      return Process(0UL, 0UL, ::GetLastError());
    }

    if (pp.job_) {
      pp.job_->add_process(pi.hProcess);
      if ((pp.flags_ & CREATE_SUSPENDED) == 0)
        ::ResumeThread(pi.hThread);
    }

    return Process(pi.hProcess, pi.hThread, NO_ERROR);
  }

};

}
