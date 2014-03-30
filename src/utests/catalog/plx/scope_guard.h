//#~def plx::ScopeGuardBase
///////////////////////////////////////////////////////////////////////////////
// plx::ScopeGuardBase
namespace plx {
class ScopeGuardBase {
 protected:
  bool dismissed_;

 public:
  void dismiss() {
    dismissed_ = true;
  }

 protected:
  ScopeGuardBase() : dismissed_(false) {}

  ScopeGuardBase(ScopeGuardBase&& other)
      : dismissed_(other.dismissed_) {
    other.dismissed_ = true;
  }
};

template <typename TFunc>
class ScopeGuardImpl : public ScopeGuardBase {
  TFunc function_;

 public:
  explicit ScopeGuardImpl(const TFunc& fn)
    : function_(fn) {}

  explicit ScopeGuardImpl(TFunc&& fn)
    : function_(std::move(fn)) {}

  ScopeGuardImpl(ScopeGuardImpl&& other)
    : ScopeGuardBase(std::move(other)),
      function_(std::move(other.function_)) {
  }

  ~ScopeGuardImpl() {
    if (!dismissed_)
      function_();
  }

 private:
  void* operator new(size_t) = delete;
};

template <typename TFunc>
ScopeGuardImpl<typename std::decay<TFunc>::type>
MakeGuard(TFunc&& fn) {
  return ScopeGuardImpl<typename std::decay<TFunc>::type>(
      std::forward<TFunc>(fn));
}

}
