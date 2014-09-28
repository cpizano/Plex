//#~def plx::VEHManager
///////////////////////////////////////////////////////////////////////////////
// plx::VEHManager 
//
namespace plx {
class VEHManager {

  struct Node {
    const DWORD code;
    const plx::Range<uint8_t> address;
    std::function<bool(EXCEPTION_RECORD*)> handler;
    Node(DWORD c, const plx::Range<uint8_t>& r, std::function<bool(EXCEPTION_RECORD*)>& f)
      : code(c), address(r), handler(f) { }
  };

  bool RunHandler(EXCEPTION_RECORD* er) {
    const uint8_t* address = nullptr;
    const DWORD code = er->ExceptionCode;

    if (code == EXCEPTION_ACCESS_VIOLATION) {
      address = reinterpret_cast<uint8_t*>(
          er->ExceptionInformation[1]);
    } else {
      address = reinterpret_cast<uint8_t*>(
          er->ExceptionAddress);
    }

    auto rl = lock_.read_lock();
    for (auto& node : handlers_) {
      if (!node.address.contains(address))
        continue;
        
      if (node.code == code) {
        if (node.handler(er))
          return true;
      }
    }

    return false;
  }

  static LONG __stdcall VecHandler(EXCEPTION_POINTERS* ep) {
    if (ep == nullptr || ep->ExceptionRecord == nullptr)
      return EXCEPTION_CONTINUE_SEARCH;
    if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
      return EXCEPTION_CONTINUE_SEARCH;
    return plx::Globals::get<VEHManager>()->RunHandler(ep->ExceptionRecord);
  }

  plx::ReaderWriterLock lock_;
  std::vector<Node> handlers_;
  void* veh_handler_;

public:
  typedef std::function<bool(EXCEPTION_RECORD*)> HandlerFn;

  VEHManager() = default;
  VEHManager(const VEHManager&) = delete;
  VEHManager& operator=(const VEHManager&) = delete;

  void add_av_handler(const plx::Range<uint8_t>& address, HandlerFn& fn) {
    auto wl = lock_.write_lock();
    handlers_.emplace_back(EXCEPTION_ACCESS_VIOLATION, address, fn);

    if (handlers_.size() == 1) {
      auto self = plx::Globals::get<VEHManager>();
      if (!self)
        throw 7;
      veh_handler_ = ::AddVectoredExceptionHandler(1, &VecHandler);
    }
  }

};

}
