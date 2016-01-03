//#~def plx::OverlappedContext
///////////////////////////////////////////////////////////////////////////////
// plx::OverlappedContext
//
namespace plx {

class ServerPipe : private plx::OvIOHandler {
  HANDLE pipe_;
  plx::OverlappedChannelHandler* handler_;
  plx::ReuseObject<plx::OverlappedContext> reuse_ovc;

private:
  ServerPipe(const ServerPipe&) = delete;
  ServerPipe& operator=(const ServerPipe&) = delete;

  explicit ServerPipe(HANDLE pipe) : pipe_(pipe) {
  }

  bool on_completed_helper(OVERLAPPED* ov, unsigned long error) {
    if (!handler_)
      return false;
    auto ovc = reinterpret_cast<plx::OverlappedContext*>(ov);
    reuse_ovc.set(ovc);

    switch (ovc->operation) {
      case plx::OverlappedContext::connect_op:
        handler_->OnConnect(ovc, error); break;
      case plx::OverlappedContext::read_op:
        handler_->OnRead(ovc, error); break;
      case plx::OverlappedContext::write_op:
        handler_->OnWrite(ovc, error); break;
      default:  __debugbreak();
    }

    reuse_ovc.reset();
    return true;
  }

  bool OnCompleted(OVERLAPPED* ov) override {
    return on_completed_helper(ov, 0);
  }

  bool OnFailure(OVERLAPPED* ov, unsigned long error) override {
    return on_completed_helper(ov, error);
  }

  bool do_async(BOOL s) {
    if (s)
      return true;
    auto gle = ::GetLastError();
    if (gle == ERROR_IO_PENDING)
      return true;
    throw plx::IOException(__LINE__, L"pipe_srv");
  }

public:
  ServerPipe()
      : pipe_(INVALID_HANDLE_VALUE) {
  }

  ServerPipe(ServerPipe&& other)
      : pipe_(INVALID_HANDLE_VALUE) {
    std::swap(other.pipe_, pipe_);
  }

  ~ServerPipe() {
    if (pipe_ != INVALID_HANDLE_VALUE)
      ::CloseHandle(pipe_);
  }

  enum Options {
    overlapped = 1,
    byte_read = 2,
    byte_write = 4
  };

  static ServerPipe Create(const wchar_t* name, int options) {
    auto type = PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE;
    if (options & overlapped) {
      type |= FILE_FLAG_OVERLAPPED;
    }
    auto mode = PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS;
    if (options & byte_write) {
      mode |= PIPE_TYPE_BYTE;
    }
    if (options & byte_read) {
      mode |= PIPE_READMODE_BYTE;
    }

    auto timeout_ms = 100UL;
    auto buf_sz = 4096UL;
    auto path = plx::FilePath::for_pipe(name);
    auto pipe = ::CreateNamedPipeW(
        path.raw(), type, mode, 1, buf_sz, buf_sz, timeout_ms, nullptr);
    if (pipe == INVALID_HANDLE_VALUE)
      throw plx::Kernel32Exception(__LINE__, Kernel32Exception::port);
    return ServerPipe(pipe);
  }

  void associate_cp(plx::CompletionPort* cp, plx::OverlappedChannelHandler* handler) {
    handler_ = handler;
    cp->add_io_handler(pipe_, this);
  }

  bool connect(void* ctx) {
    auto ovc = ctx ?
        reuse_ovc.get(plx::OverlappedContext::connect_op, ctx) : nullptr;
    return do_async(::ConnectNamedPipe(pipe_, ovc));
  }

  bool read(plx::Range<uint8_t> buf, void* ctx) {
    auto ovc = ctx ?
        reuse_ovc.get(plx::OverlappedContext::read_op, ctx, buf) : nullptr;
    return do_async(::ReadFile(pipe_, buf.start(), plx::To<DWORD>(buf.size()), NULL, ovc));
  }

  bool write(plx::Range<uint8_t> buf, void* ctx) {
    auto ovc = ctx ?
        reuse_ovc.get(plx::OverlappedContext::write_op, ctx, buf) : nullptr;
    return do_async(::WriteFile(pipe_, buf.start(), plx::To<DWORD>(buf.size()), NULL, ovc));
  }

  bool disconnect() {
    return ::DisconnectNamedPipe(pipe_) ? true : false;
  }
};

}
