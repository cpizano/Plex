//#~def plx::Globals
///////////////////////////////////////////////////////////////////////////////
// plx::Globals
//
namespace plx {
template <class T> struct ServiceNumber;
class TestService; template <> struct ServiceNumber<TestService> { enum { id = 0 }; };

class Globals {
  void* services_[__LINE__ - 9];

  static void* raw_get(int id, Globals* ctx = nullptr) {
    static Globals* plx_globals = ctx;
    return plx_globals->services_[id];
  }

public:
  Globals() {
    memset(services_, 0, sizeof(services_));
    raw_get(0, this);
  }

  template <typename Svc>
  void add_service(Svc* svc) {
    const int id = ServiceNumber<Svc>::id;
    services_[id] = svc;
  }

  template <typename Svc>
  static Svc* get() {
    const int id = ServiceNumber<Svc>::id;
    return reinterpret_cast<Svc*>(raw_get(id));
  }

};

}
