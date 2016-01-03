//#~def plx::JobObject
///////////////////////////////////////////////////////////////////////////////
// plx::JobObject
//
namespace plx {

class JobObject {
  HANDLE handle_;
  unsigned long status_;

private:
  JobObject(HANDLE handle, unsigned long status)
    : handle_(handle), status_(status) {}

  JobObject(const JobObject&) = delete;
  JobObject& operator=(const JobObject&) = delete;

public:
  JobObject() : handle_(0UL) {}

  JobObject(JobObject&& jobo) : handle_(0UL), status_(0UL) {
    std::swap(jobo.handle_, handle_);
    std::swap(jobo.status_, status_);
  }

  ~JobObject() {
    if (handle_) {
      if (!::CloseHandle(handle_))
        __debugbreak();
    }
  }

  static JobObject Create(const wchar_t* name,
    const plx::JobObjectLimits& limits,
    const plx::JobObjecNotification* notification) {
    auto job = ::CreateJobObjectW(nullptr, name);
    auto gle = ::GetLastError();
    if (!job)
      return JobObject(0UL, gle);

    if (gle != ERROR_ALREADY_EXISTS) {
      limits.config(job);
    }
    if (notification)
      notification->config(job);
    return JobObject(job, gle);
  }

  unsigned status() const { return status_; }

  bool add_process(HANDLE process) {
    return ::AssignProcessToJobObject(handle_, process) ? true : false;
  }

  bool is_valid() const { return handle_ != 0UL; }

};

}
