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
    read_only = PAGE_READONLY | SEC_COMMIT,
    read_write = PAGE_READWRITE | SEC_COMMIT,
  };

  SharedSection(const std::wstring name, SP protect, size_t size)
      : mapping_(0), existing_(false) {
    LARGE_INTEGER li;
    li.QuadPart = size;
    mapping_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                   nullptr,
                                   protect,
                                   li.HighPart, li.LowPart,
                                   name.c_str());
    if (!mapping_)
      throw plx::Kernel32Exception(__LINE__, plx::Kernel32Exception::memory);
    if (::GetLastError() == ERROR_ALREADY_EXISTS)
      existing_ = true;
  }

  bool existing() const { return existing_; }

  ~SharedSection() {
    ::CloseHandle(mapping_);
  }

  SharedSection(const SharedMemory&) = delete;
  SharedSection& operator=(const SharedMemory&) = delete;

  plx::SharedMemory map(size_t start, size_t size, SP protect) const {
    auto p = protect == read_only ? SharedMemory::map_r : SharedMemory::map_rw;
    return SharedMemory(mapping_, start, size, p);
  }
};
}
