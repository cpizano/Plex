//#~def plx::DemandPagedMemory
///////////////////////////////////////////////////////////////////////////////
// plx::DemandPagedMemory 
//
namespace plx {
class DemandPagedMemory {
  size_t page_faults_;
  size_t block_size_;
  plx::Range<uint8_t> range_;
  
public:
  DemandPagedMemory(size_t max_bytes, size_t block_pages)
      : page_faults_(0),
        block_size_(block_pages * 4096),
        range_(reinterpret_cast<uint8_t*>(
            ::VirtualAlloc(NULL, max_bytes, MEM_RESERVE, PAGE_NOACCESS)), max_bytes) {
    if (!range_.start())
      throw plx::MemoryException(__LINE__, 0);
    auto veh_man = plx::Globals::get<plx::VEHManager>();
    plx::VEHManager::HandlerFn fn =
        std::bind(&DemandPagedMemory::page_fault, this, std::placeholders::_1);
    veh_man->add_av_handler(range_, fn);
  }

  ~DemandPagedMemory() {
    auto veh_man = plx::Globals::get<plx::VEHManager>();
    veh_man->remove_av_handler(range_);
    ::VirtualFree(range_.start(), 0, MEM_RELEASE);
  }

  plx::Range<uint8_t> get() { return range_; }

  size_t page_faults() const { return page_faults_; }

private:
  bool page_fault(EXCEPTION_RECORD* er) {
    ++page_faults_;
    auto addr = reinterpret_cast<uint8_t*>(er->ExceptionInformation[1]);
    auto alloc_sz = std::min(block_size_, size_t(range_.end() - addr));
    auto new_addr = ::VirtualAlloc(addr, alloc_sz, MEM_COMMIT, PAGE_READWRITE);
    return (new_addr != nullptr);
  }
};
}
