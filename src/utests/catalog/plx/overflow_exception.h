//#~def plx::OverflowException
///////////////////////////////////////////////////////////////////////////////
// plx::OverflowException
// kind_ : Type of overflow, positive or negative.
namespace plx {
class OverflowException : public plx::Exception {
  plx::OverflowKind kind_;

public:
  OverflowException(int line, plx::OverflowKind kind)
      : Exception(line, "Overflow"), kind_(kind) {
    PostCtor();
  }
  plx::OverflowKind kind() const { return kind_; }
};
}
