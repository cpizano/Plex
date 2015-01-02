//#~def plx::Globals
///////////////////////////////////////////////////////////////////////////////
// plx::Globals
//
namespace plx {
template <class T> struct ServiceNumber;
class TestService; template <> struct ServiceNumber<TestService> { enum { id = 0 }; };
class VEHManager;  template <> struct ServiceNumber<VEHManager>  { enum { id = 1 }; }; 

class Globals {
  void* services_[2];

  static void* raw_get(int id, void** svcs = nullptr) {
    static void** services = svcs;
    return services[id];
  }

  Globals(const Globals&) = delete;
  Globals& operator=(const Globals&) = delete;

public:
  Globals() {
    memset(services_, 0, sizeof(services_));
    raw_get(0, services_);
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
