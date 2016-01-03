//#~def plx::ProcessParams
///////////////////////////////////////////////////////////////////////////////
// plx::ProcessParams
//
namespace plx {

class ProcessParams {
  bool inherit_;
  unsigned long flags_;
  void* env_;
  plx::JobObject* job_;

  friend class Process;

public:
  ProcessParams(bool inherit, unsigned long flags)
    : inherit_(inherit),
      flags_(flags),
      env_(nullptr) {
  }

  void set_enviroment(void* env) { env_ = env; }
  void set_job(plx::JobObject* job) { job_ = job;  }
};

}
