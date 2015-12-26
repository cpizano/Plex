//#~def plx::DispatchResult
//#~def plx::Message
///////////////////////////////////////////////////////////////////////////////
// plx::Message and MsgMakeNew
// 
namespace plx {

enum class DispatchResult {
  dispatch_ok,
  short_buffer,
  bad_buffer,
  handler_abort,
  no_handler
};

#define MsgALIGNED alignas(16) Message

struct MsgALIGNED {
  static const uint32_t msg_bom = 0x5ca1ab1e;

  uint32_t bom;
  uint32_t id;
  uint32_t size;

  static DispatchResult validate_header(const plx::Range<uint8_t>& r, uint32_t* id) {
    if (r.size() < sizeof(Message))
      return DispatchResult::short_buffer;
    auto m = reinterpret_cast<Message*>(r.start());
    if (m->bom != msg_bom)
      return DispatchResult::bad_buffer;
    *id = m->id;
    return DispatchResult::dispatch_ok;
  }

  static void init_header(Message* m, uint32_t id, uint32_t size) {
    m->bom = msg_bom;
    m->id = id;
    m->size = size;
  }
};

#undef MsgALIGNED

template<typename M>
M* MsgMakeNew() {
  auto m = new M();
  Message::init_header(m, M::M_ID, sizeof(M));
  return m;
}

template<typename M> bool MsgHandler(M* m, void* ctx);

}