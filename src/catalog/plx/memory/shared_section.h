//#~def plx::SharedSection
///////////////////////////////////////////////////////////////////////////////
// plx::SharedSection 
//
namespace plx {
class SharedSection {
  HANDLE mapping_;
  bool existing_;

public:
  enum SP {
    read_only,
    read_write
  };

  SharedSection() : mapping_(0), existing_(false) {}

  SharedSection(const std::wstring name, SP protect, size_t size)
      : mapping_(0), existing_(false) {
    LARGE_INTEGER li;
    li.QuadPart = size;
    auto p = protect == read_only ? PAGE_READONLY : PAGE_READWRITE;
    mapping_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                   nullptr,
                                   p | SEC_COMMIT,
                                   li.HighPart, li.LowPart,
                                   name.c_str());
    if (!mapping_)
      throw plx::Kernel32Exception(__LINE__, plx::Kernel32Exception::memory);
    if (::GetLastError() == ERROR_ALREADY_EXISTS)
      existing_ = true;
  }

  SharedSection(SP protect, const std::wstring name)
      : mapping_(0), existing_(false) {
    // Note the use of FILE_MAP constants, unlike CreateFileMapping.
    auto p = protect == read_only ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE;
    mapping_ = ::OpenFileMapping(p, FALSE, name.c_str());
    if (mapping_)
      existing_ = true;
  }

  bool existing() const { return existing_; }

  ~SharedSection() {
    if (mapping_)
      ::CloseHandle(mapping_);
  }

  SharedSection(const SharedMemory&) = delete;
  SharedSection& operator=(const SharedMemory&) = delete;

  SharedSection& operator=(SharedSection&& other) {
    std::swap(other.mapping_, mapping_);
    std::swap(other.existing_, existing_);
    return *this;
  }

  plx::SharedMemory map(size_t start, size_t size, SP protect) const {
    auto p = protect == read_only ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE;
    return SharedMemory(mapping_, start, size, p);
  }
};
}
