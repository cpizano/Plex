//#~def plx::CompletionPort
//#~def plx::OvIOHandler
///////////////////////////////////////////////////////////////////////////////
// plx::OverlappedContext
//
namespace plx {

class OvIOHandler {
public:
  virtual bool OnCompleted(OVERLAPPED* ov) = 0;
  virtual bool OnFailure(OVERLAPPED* ov, unsigned long error) = 0;
};

class CompletionPort {
  HANDLE port_;

private:
  CompletionPort() = delete;
  CompletionPort(const CompletionPort&) = delete;
  CompletionPort& operator=(const CompletionPort&) = delete;

public:

  explicit CompletionPort(unsigned long concurrent)
    : port_(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, concurrent)) {
    if (!port_)
      throw plx::Kernel32Exception(__LINE__, plx::Kernel32Exception::port);
  }

  ~CompletionPort() {
    ::CloseHandle(port_);
  }

  void add_handler(HANDLE handle, OvIOHandler* handler) {
    auto h = ::CreateIoCompletionPort(handle, port_, ULONG_PTR(handler), 0);
    if (!h)
      throw plx::Kernel32Exception(__LINE__, plx::Kernel32Exception::port);;
  }

  void release_waiter() {
    ::PostQueuedCompletionStatus(port_, 0, 1, nullptr);
  }

  enum WaitResult {
    op_exit,
    op_ok,
    op_timeout,
    op_error
  };

  WaitResult wait_for_op(unsigned long timeout) {
    unsigned long bytes;
    ULONG_PTR key;
    OVERLAPPED* ov;

    // return  | ov | bytes 
    // ----------------------------------------------------
    //   false |  0 | na   The call to GQCS failed, and no data was dequeued from the IO port.
    //                     Usually means wrong parameters.
    //   false |  x | na   The call to GQCS failed, but data was read or written. There is an
    //                     error condition on the underlying HANDLE. Usually seen when the other
    //                     end has been forcibly closed but there's still data in the send or
    //                     receive queue.
    //   true  |  0 |  y   Only possible via PostQueuedCompletionStatus(). Can use y or key to
    //                     communicate condtions.
    //   true  |  x |  0   End of file for a file HANDLE, or the connection has been gracefully
    //                     closed (for network connections).
    //   true  |  x |  y   Sucess. y bytes of data have been transferred.

    if (!::GetQueuedCompletionStatus(port_, &bytes, &key, &ov, timeout)) {
      if (!ov) {
        if (::GetLastError() == WAIT_TIMEOUT)
          return op_timeout;
        throw plx::IOException(__LINE__, nullptr);
      } else if (key) {
        return reinterpret_cast<OvIOHandler*>(key)->OnFailure(ov, ::GetLastError()) ? op_ok : op_error;
      }
    } else if (!ov) {
      return op_exit;
    } else if (key) {
      return reinterpret_cast<OvIOHandler*>(key)->OnCompleted(ov) ? op_ok : op_error;
    }
    throw plx::IOException(__LINE__, nullptr);
  }

};


}
