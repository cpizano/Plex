//#~def plx::FilesInfo
///////////////////////////////////////////////////////////////////////////////
// plx::FilesInfo
//
namespace plx {
#pragma comment(user, "plex.define=plex_vista_support")
class FilesInfo {
private:
  FILE_ID_BOTH_DIR_INFO* info_;
  std::unique_ptr<unsigned char[]> data_;
  mutable bool done_;

public:
  static FilesInfo FromDir(plx::File& file) {
    if (file.status() != (plx::File::directory | plx::File::existing))
      throw plx::IOException(__LINE__, nullptr);
    DWORD size = 128 * 32;
    auto data = std::make_unique<unsigned char[]>(size);
    if (!::GetFileInformationByHandleEx(file.handle_, FileIdBothDirectoryInfo, &data[0], size))
      throw plx::IOException(__LINE__, nullptr);
    return FilesInfo(data.release());
  }

  void first() {
    done_ = false;
    info_ = reinterpret_cast<FILE_ID_BOTH_DIR_INFO*>(&data_[0]);
  }

  void next() {
    info_ =reinterpret_cast<FILE_ID_BOTH_DIR_INFO*>(
        ULONG_PTR(info_) + info_->NextEntryOffset);
  }

  bool done() const {
    if (done_)
      return true;
    if (!info_->NextEntryOffset)
      done_ = true;
    return false;
  }

  const wchar_t* file_name() const {
    return info_->FileName;
  }

private:
  FilesInfo(unsigned char* data) 
      : info_(nullptr), data_(data), done_(false) {
  }
};

}
