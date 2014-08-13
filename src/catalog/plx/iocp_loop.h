//#~def plx::IOCPLoopProxy
//#~def plx::IOCPLoop
///////////////////////////////////////////////////////////////////////////////
// plx::IOCPLoop  (message loop basd on IO completion ports)
// plx::IOCPProxy (the way to post tasks to the  the IOCPLoop)
//
namespace plx {

class IOCPLoopProxy {
  HANDLE port_;
  friend class IOCPLoop;

  IOCPLoopProxy(HANDLE port) : port_(port) {
  }

public:
  bool PostTask(const std::function<void()>& closure) {
    auto cc = new std::function<void()>(closure);
    if (!::PostQueuedCompletionStatus(port_, 0, ULONG_PTR(cc), NULL)) {
      delete cc;
      return false;
    }
    return true;
  }

  bool PostQuitTask() {
    return TRUE == ::PostQueuedCompletionStatus(port_, 0, 0, NULL);
  }
};

class IOCPLoop {
  HANDLE port_;
  unsigned int calls_;

public:
  IOCPLoop() : port_(nullptr), calls_(0) {
    port_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
  }

  IOCPLoop(const IOCPLoop&) = delete;

  ~IOCPLoop() {
    ::CloseHandle(port_);
  }

  IOCPLoopProxy MakeProxy() {
    return IOCPLoopProxy(port_);
  }

  unsigned int call_count() const {
    return calls_;
  }

  void Run(unsigned long timeout_ms) {
    DWORD bytes = 0;
    OVERLAPPED* ov;
    ULONG_PTR key;
    while (true) {
      ::GetQueuedCompletionStatus(port_, &bytes, &key, &ov, timeout_ms);
      if (!key)
        break;
      auto closure = reinterpret_cast<std::function<void ()>*>(key);
      (*closure)();
      ++calls_;
      delete closure;
    }
  }

};

}
