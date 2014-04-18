//#~def plx::InvalidParamException
///////////////////////////////////////////////////////////////////////////////
// plx::IOException
// error_code_ : The win32 error code of the last operation.
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
