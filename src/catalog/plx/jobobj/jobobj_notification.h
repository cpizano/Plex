//#~def plx::JobObjecNotification
///////////////////////////////////////////////////////////////////////////////
// plx::JobObjecNotification
//
namespace plx {

class JobObjecNotification {
  plx::CompletionPort* cp_;
  plx::JobObjEventHandler* handler_;

  friend class JobObject;
  void config(HANDLE job) const {
    if (cp_) {
      JOBOBJECT_ASSOCIATE_COMPLETION_PORT info = { handler_, cp_->handle() };
      ::SetInformationJobObject(
        job, JobObjectAssociateCompletionPortInformation, &info, sizeof(info));
    }
  }

public:
  JobObjecNotification() : cp_(nullptr), handler_(nullptr) {}
  JobObjecNotification(plx::CompletionPort* cp, JobObjEventHandler* handler)
    : cp_(cp), handler_(handler) {}

  plx::CompletionPort::WaitResult wait_for_event(unsigned long timeout) {
    plx::RawGQCPS raw;
    auto rv = cp_->wait_raw(timeout, &raw);
    if (rv != plx::CompletionPort::op_ok)
      return rv;
    auto handler = reinterpret_cast<JobObjEventHandler*>(raw.key);
    if (!handler)
      return plx::CompletionPort::op_error;

    auto pid = plx::To<unsigned int>(reinterpret_cast<UINT_PTR>(raw.ov));

    switch (raw.bytes) {
      case JOB_OBJECT_MSG_END_OF_JOB_TIME:
        handler->TimeLimit(pid); break;
      case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
        handler->TimeLimit(pid); break;
      case JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT:
        break;
      case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
        handler->ActiveCountZero(); break;
      case JOB_OBJECT_MSG_NEW_PROCESS:
        handler->NewProcess(pid); break;
      case JOB_OBJECT_MSG_EXIT_PROCESS:
        handler->NormalExit(pid, 0); break;
      case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
        handler->AbnormalExit(pid, 0); break;
      case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
        handler->MemoryLimit(pid); break;
      case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
        handler->MemoryLimit(pid); break;
      case JOB_OBJECT_MSG_NOTIFICATION_LIMIT:
      case JOB_OBJECT_MSG_JOB_CYCLE_TIME_LIMIT:
      default:
        break;
    }
    return rv;
  }
};

}
