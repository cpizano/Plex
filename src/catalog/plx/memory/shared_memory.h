//#~def plx::SharedMemory
///////////////////////////////////////////////////////////////////////////////
// plx::SharedMemory
//
namespace plx {
class SharedMemory {
  plx::Range<uint8_t> range_;
  friend class SharedSection;

public:
  SharedMemory() = default;

  SharedMemory(SharedMemory&& other) : range_(other.range_) {
    other.range_.clear();
  }

  SharedMemory(const SharedMemory&) = delete;
  SharedMemory& operator=(const SharedMemory&) = delete;

  SharedMemory& operator=(SharedMemory&& other) {
    std::swap(other.range_, range_);
    return *this;
  }

  ~SharedMemory() {
    if (!range_.empty())
      ::UnmapViewOfFile(range_.start());
  }

  plx::Range<uint8_t> range() const { return range_; }

protected:
  enum MP {
    map_r = SECTION_MAP_READ,
    map_rw = SECTION_MAP_READ | SECTION_MAP_WRITE
  };

  SharedMemory(HANDLE section, size_t start, size_t size, MP protect) {
    LARGE_INTEGER li;
    li.QuadPart = start;
    auto addr = reinterpret_cast<uint8_t*>(
        ::MapViewOfFileEx(section, protect, li.HighPart, li.LowPart, size, nullptr));
    if (!addr)
      throw plx::Kernel32Exception(__LINE__, plx::Kernel32Exception::memory);
    range_= plx::Range<uint8_t>(addr, size);
  }
};
}
