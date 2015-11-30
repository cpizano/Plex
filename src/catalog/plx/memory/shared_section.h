//#~def plx::SharedSection
///////////////////////////////////////////////////////////////////////////////
// plx::SharedSection 
//
namespace plx {
class SharedSection {
  HANDLE mapping_;
  bool existing_;

private:
  SharedSection(const SharedSection&) = delete;
  SharedSection& operator=(const SharedSection&) = delete;

  SharedSection(HANDLE mapping, bool existing)
      : mapping_(mapping),
        existing_(existing) {
  }

public:
  enum SP {
    read_only,
    read_write
  };

  SharedSection() 
      : mapping_(0),
        existing_(false) {
  }

  SharedSection(SharedSection&& section)
      : mapping_(0),
        existing_(false) {
    std::swap(section.mapping_, mapping_);
    std::swap(section.existing_, existing_);
  }

  ~SharedSection() {
    if (mapping_)
      ::CloseHandle(mapping_);
  }

  SharedSection& operator=(SharedSection&& section) {
    std::swap(section.mapping_, mapping_);
    std::swap(section.existing_, existing_);
    return *this;
  }

  static SharedSection Create(const std::wstring& name, SP protect, size_t size) {
    LARGE_INTEGER li;
    li.QuadPart = size;
    auto p = protect == read_only ? PAGE_READONLY : PAGE_READWRITE;
    auto mapping = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                   nullptr,
                                   p | SEC_COMMIT,
                                   li.HighPart, li.LowPart,
                                   name.c_str());
    if (!mapping)
      throw plx::Kernel32Exception(__LINE__, plx::Kernel32Exception::memory);
    
    return SharedSection(mapping, ::GetLastError() == ERROR_ALREADY_EXISTS);
  }

  static SharedSection Open(const std::wstring& name, SP protect) {
    // Note the use of FILE_MAP constants, unlike CreateFileMapping.
    auto p = protect == read_only ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE;
    auto mapping = ::OpenFileMapping(p, FALSE, name.c_str());
    return SharedSection(mapping, mapping ? true : false);
  }

  bool existing() const { return existing_; }

  plx::SharedMemory map(size_t start, size_t size, SP protect) const {
    auto p = protect == read_only ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE;
    return SharedMemory(mapping_, start, size, p);
  }
};
}
