//#~def plx::JobObjEventHandler
///////////////////////////////////////////////////////////////////////////////
// plx::JobObjEventHandler
//
namespace plx {

class JobObjEventHandler {
public:
  virtual void AbnormalExit(unsigned int pid, unsigned long error) = 0;
  virtual void NormalExit(unsigned int pid, unsigned long status) = 0;
  virtual void NewProcess(unsigned int pid) = 0;
  virtual void ActiveCountZero() = 0;
  virtual void ActiveProcessLimit() = 0;
  virtual void MemoryLimit(unsigned int pid) = 0;
  virtual void TimeLimit(unsigned int pid) = 0;
};

}
