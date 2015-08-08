//#~def plx::SharedSection
///////////////////////////////////////////////////////////////////////////////
// plx::SharedSection 
//
namespace plx {
class SharedSection {
  HANDLE mapping_;

public:
  enum SP {
    read_only = PAGE_READONLY,
    read_write = PAGE_READWRITE,
  };

  SharedSection(const std::wstring name, SP protect, size_t size) : mapping_(0) {
    LARGE_INTEGER li;
    li.QuadPart = size;
    mapping_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                   nullptr,
                                   protect,
                                   li.HighPart, li.LowPart,
                                   name.c_str());
    if (!mapping_)
      throw plx::Kernel32Exception(__LINE__, plx::Kernel32Exception::memory);
  }

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
