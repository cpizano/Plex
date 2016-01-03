//#~def plx::ProcessParams
///////////////////////////////////////////////////////////////////////////////
// plx::ProcessParams
//
namespace plx {

class ProcessParams {
  bool inherit_;
  unsigned long flags_;
  plx::JobObject* job_;
  void* env_;

  friend class Process;

public:
  ProcessParams(bool inherit, unsigned long flags)
    : inherit_(inherit),
      flags_(flags),
      job_(nullptr),
      env_(nullptr) {
  }

  void set_enviroment(void* env) { env_ = env; }
  void set_job(plx::JobObject* job) { job_ = job;  }
};

}
