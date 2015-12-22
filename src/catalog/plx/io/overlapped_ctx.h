//#~def plx::OverlappedContext
///////////////////////////////////////////////////////////////////////////////
// plx::OverlappedContext
//
namespace plx {

struct OverlappedContext : public OVERLAPPED {
  enum OverlappedOp {
    none_op,
    connect_op,
    read_op,
    write_op,
    disconnect_op
  };

  OverlappedOp operation;
  void* ctx;
  plx::Range<uint8_t> data;

  OverlappedContext(OverlappedOp op, void* ctx, plx::Range<uint8_t> data)
    : OVERLAPPED({}), operation(op), ctx(ctx), data(data) {
  }

  ~OverlappedContext() {
    if (hEvent)
      ::CloseHandle(hEvent);
  }

  OverlappedContext(const OverlappedContext&) = delete;
  OverlappedContext operator=(const OverlappedContext&) = delete;

  OverlappedContext* reuse(OverlappedOp op, void* ctx, plx::Range<uint8_t> data) {
    this->~OverlappedContext();
    return new (this) OverlappedContext(op, ctx, data);
  }

  size_t number_of_bytes() const {
    return InternalHigh;
  }

  size_t status_code() const {
    return Internal;
  }

  plx::Range<uint8_t> valid_data() {
    return plx::Range<uint8_t>(data.start(), number_of_bytes());
  }

  void make_event() {
    hEvent = ::CreateEvent(nullptr, true, false, nullptr);
  }
};

}
