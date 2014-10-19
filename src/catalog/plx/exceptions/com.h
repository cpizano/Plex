//#~def plx::ComException
///////////////////////////////////////////////////////////////////////////////
// plx::ComException (thrown when COM fails)
//
namespace plx {
class ComException : public plx::Exception {
  HRESULT hr_;

public:
  ComException(int line, HRESULT hr)
      : Exception(line, "COM failure"), hr_(hr) {
    PostCtor();
  }

  HRESULT hresult() const {
    return hr_;
  }

};
}
