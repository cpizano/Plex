//#~def plx::FileParams
///////////////////////////////////////////////////////////////////////////////
// plx::FileParams
// access_  : operations allowed to the file.
// sharing_ : operations others can do to the file.
// disposition_ : if the file is reset or opened as is.
// flags_ : hints or special modes go here.
//
namespace plx {
class FileParams {
private:
  DWORD access_;
  DWORD sharing_;
  DWORD disposition_;
  DWORD attributes_;  // For existing files these are generally ignored.
  DWORD flags_;       // For existing files these are generally combined.
  DWORD sqos_;
  friend class File;

public:
  static const DWORD kShareNone = 0;
  static const DWORD kShareAll = FILE_SHARE_DELETE |
                                 FILE_SHARE_READ |
                                 FILE_SHARE_WRITE;
  FileParams()
    : access_(0),
      sharing_(kShareAll),
      disposition_(OPEN_EXISTING),
      attributes_(FILE_ATTRIBUTE_NORMAL),  
      flags_(0),  
      sqos_(0) {
  }

  FileParams(DWORD access,
             DWORD sharing,
             DWORD disposition,
             DWORD attributes,
             DWORD flags,
             DWORD sqos)
    : access_(access),
      sharing_(sharing),
      disposition_(disposition),
      attributes_(attributes),
      flags_(flags),
      sqos_(sqos) {
    // Don't allow generic access masks.
    if (access & 0xF0000000)
      throw plx::InvalidParamException(__LINE__, 1);
  }

  bool can_modify() const {
    return (access_ & (FILE_WRITE_DATA |
                       FILE_WRITE_ATTRIBUTES |
                       FILE_WRITE_EA |
                       FILE_APPEND_DATA)) != 0;
  }

  bool exclusive() const {
    return (sharing_ == 0);
  }

  static FileParams Append_SharedRead() {
    return FileParams(FILE_APPEND_DATA, FILE_SHARE_READ,
                      OPEN_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, 0, 0);
  }

  static FileParams Read_SharedRead() {
    return FileParams(FILE_GENERIC_READ, FILE_SHARE_READ,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, 0, 0);
  }

  static FileParams ReadWrite_SharedRead(DWORD disposition) {
    return FileParams(FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ,
                      disposition,
                      FILE_ATTRIBUTE_NORMAL, 0, 0);
  }
};
}

