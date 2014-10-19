//#~def plx::User32Exception
///////////////////////////////////////////////////////////////////////////////
// plx::User32Exception (thrown by user32 functions)
//
namespace plx {
class User32Exception : public plx::Exception {
public:
  enum Kind {
    wclass,
    window,
    menu,
    device,
    cursor,
    icon,
    accelerator
  };

  User32Exception(int line, Kind type)
      : Exception(line, "user 32"), type_(type) {
    PostCtor();
  }

  Kind type() const {
    return type_;
  }

private:
  Kind type_;
};
}
