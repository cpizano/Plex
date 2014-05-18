//#~def plx::InvalidParamException
///////////////////////////////////////////////////////////////////////////////
// plx::InvalidParamException
// parameter_ : the position of the offending parameter, zero if unknown.
//
namespace plx {
class InvalidParamException : public plx::Exception {
  int parameter_;

public:
  InvalidParamException(int line, int parameter)
      : Exception(line, "Invalid parameter"), parameter_(parameter) {
    PostCtor();
  }
  int Parameter() const { return parameter_; }
};
}
