//#~def plx::CodecException
///////////////////////////////////////////////////////////////////////////////
// plx::CodecException
// bytes_ : The 16 bytes or less that caused the issue.
namespace plx {
class CodecException : public plx::Exception {
  unsigned char bytes_[16];
  size_t count_;

public:
  CodecException(int line, const plx::Range<const unsigned char>* br)
      : Exception(line, "Codec exception"), count_(0) {
    if (br)
      count_ = br->CopyToArray(bytes_);
    PostCtor();
  }

};
}
