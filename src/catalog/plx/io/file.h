//#~def plx::File
///////////////////////////////////////////////////////////////////////////////
// plx::File
//
namespace plx {
class File {
  HANDLE handle_;
  unsigned int  status_;
  friend class FilesInfo;

private:
  File(HANDLE handle,
       unsigned int status)
    : handle_(handle),
      status_(status) {
  }

  File();
  File(const File&);
  File& operator=(const File&);

public:
  enum Status {
    none              = 0,
    brandnew          = 1,   // 
    existing          = 2,
    delete_on_close   = 4,
    directory         = 8,   // $$$ just use backup semantics?
    exclusive         = 16,
    readonly          = 32,
    information       = 64,
  };

  File(File&& file) 
    : handle_(INVALID_HANDLE_VALUE),
      status_(none) {
    std::swap(file.handle_, handle_);
    std::swap(file.status_, status_);
  }

  static File Create(const plx::FilePath& path,
                     const plx::FileParams& params,
                     const plx::FileSecurity& security) {
    HANDLE file = ::CreateFileW(path.path_.c_str(),
                                params.access_,
                                params.sharing_,
                                security.sattr_,
                                params.disposition_,
                                params.attributes_ | params.flags_ | params.sqos_,
                                0);
    DWORD gle = ::GetLastError();
    unsigned int status = none;

    if (file != INVALID_HANDLE_VALUE) {

      switch (params.disposition_) {
        case OPEN_EXISTING:
        case TRUNCATE_EXISTING:
          status |= existing; break;
        case CREATE_NEW:
          status |= (gle == ERROR_FILE_EXISTS) ? existing : brandnew; break;
        case CREATE_ALWAYS:
        case OPEN_ALWAYS:
          status |= (gle == ERROR_ALREADY_EXISTS) ? existing : brandnew; break;
        default:
          __debugbreak();
      };

      if (params.flags_ == FILE_FLAG_BACKUP_SEMANTICS) status |= directory;
      if (params.flags_ & FILE_FLAG_DELETE_ON_CLOSE) status |= delete_on_close;
      if (params.sharing_ == 0) status |= exclusive;
      if (params.access_ == 0) status |= information;
      if (params.access_ == GENERIC_READ) status |= readonly;
    }

    return File(file, status);
  }

  ~File() {
    if (handle_ != INVALID_HANDLE_VALUE) {
      if (!::CloseHandle(handle_)) {
        throw IOException(__LINE__, nullptr);
      }
    }
  }

  // $$ this ignores the volume id, so technically incorrect.
  long long get_unique_id() {
    BY_HANDLE_FILE_INFORMATION bhfi;
    if (!::GetFileInformationByHandle(handle_, &bhfi))
      throw IOException(__LINE__, nullptr);
    LARGE_INTEGER li = { bhfi.nFileIndexLow, bhfi.nFileIndexHigh };
    return li.QuadPart;
  }

  unsigned int status() const {
    return status_;
  }

  bool is_valid() const {
    return (handle_ != INVALID_HANDLE_VALUE);
  }

  size_t size_in_bytes() const {
    LARGE_INTEGER li = {0};
    ::GetFileSizeEx(handle_, &li);
    return li.QuadPart;
  }

  size_t read(plx::Range<char>& mem, unsigned int from) {
    OVERLAPPED ov = {0};
    ov.Offset = from;
    DWORD read = 0;
    if (!::ReadFile(handle_, mem.start(), static_cast<DWORD>(mem.size()),
                    &read, &ov))
      return 0;
    return read;
  }

  size_t write(const plx::Range<const char>& mem, int from = -1) {
    return write(mem.start(), mem.size(), from);
  }

  size_t write(const plx::Range<char>& mem, int from = -1) {
    return write(mem.start(), mem.size(), from);
  }

  size_t write(const char* buf, size_t len, int from) {
    OVERLAPPED ov = {0};
    ov.Offset = from;
    DWORD written = 0;
    if (!::WriteFile(handle_, buf, static_cast<DWORD>(len),
                     &written, (from < 0) ? nullptr : &ov))
      return 0;
    return written;
  }
};
}
