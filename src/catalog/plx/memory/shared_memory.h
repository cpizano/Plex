//#~def plx::SharedMemory
//#~def plx::SharedSection
///////////////////////////////////////////////////////////////////////////////
// plx::SharedMemory, SharedSection 
//
namespace plx {
class SharedMemory {
  plx::Range<uint8_t> range_;
  friend class SharedSection;

public:
  SharedMemory(SharedMemory&& other) : range_(other.range_) {
    other.range_.clear();
  }

  SharedMemory(const SharedMemory&) = delete;
  SharedMemory& operator=(const SharedMemory&) = delete;

  ~SharedMemory() {
    if (!range_.empty())
      ::UnmapViewOfFile(range_.start());
  }

  plx::Range<uint8_t> range() { return range_; }

protected:
  SharedMemory(const plx::Range<uint8_t>& range)
      : range_(range) {
  }

};

class SharedSection {
  HANDLE mapping_;

public:
  enum SP {
    section_r = PAGE_READONLY,
    section_rw = PAGE_READWRITE,
  };

  enum MP {
    map_r = SECTION_MAP_READ,
    map_rw = SECTION_MAP_READ | SECTION_MAP_WRITE
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
      throw 1;
  }

  ~SharedSection() {
    ::CloseHandle(mapping_);
  }

  SharedSection(const SharedMemory&) = delete;
  SharedSection& operator=(const SharedMemory&) = delete;

  SharedMemory map(size_t start, size_t size, MP protect) {
    LARGE_INTEGER li;
    li.QuadPart = start;
    auto addr = reinterpret_cast<uint8_t*>(
        ::MapViewOfFileEx(mapping_, protect, li.HighPart, li.LowPart, size, nullptr));
    if (!addr)
      throw 2;
    return SharedMemory(plx::Range<uint8_t>(addr, size));
  }

};

}
