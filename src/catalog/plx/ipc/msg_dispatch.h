//#~def plx::DispatchMap
//#~def plx::MsgDispatch
///////////////////////////////////////////////////////////////////////////////
// plx::MsgDispatch
// 
namespace plx {

struct MsgDispatch {
  void* maker;
  void* handler;
  void* ctx;
};

using DispatchMap = std::map<uint32_t, MsgDispatch>;

template<typename M>
M* MsgFromMem(plx::Range<uint8_t> r) {
  if (r.size() < sizeof(M))
    return nullptr;
  auto m = new (r.start()) M;
  return (m->id == M::M_ID) ? m : nullptr;
}

template <typename M>
void AddToDispatchMap(DispatchMap& dm, void* ctx) {
  dm[M::M_ID] = { &MsgFromMem<M>, &MsgHandler<M>, ctx };
}

plx::DispatchResult MsgDispatch(DispatchMap dm, plx::Range<uint8_t>& r) {
  using Make = decltype(MsgFromMem<Message>);
  using Handle = decltype(MsgHandler<Message>);

  uint32_t id = 0;
  auto res = Message::validate_header(r, &id);
  if (res != DispatchResult::dispatch_ok)
    return res;

  auto e = dm.find(id);
  if (e == dm.end())
    return DispatchResult::no_handler;

  auto msg = reinterpret_cast<Make*>(e->second.maker)(r);
  if (!msg)
    return DispatchResult::bad_buffer;
  if (!reinterpret_cast<Handle*>(e->second.handler)(msg, e->second.ctx))
    return DispatchResult::handler_abort;
  r.advance(msg->size);
  return DispatchResult::dispatch_ok;
}

}