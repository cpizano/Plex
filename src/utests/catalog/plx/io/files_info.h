//#~def plx::FilesInfo
///////////////////////////////////////////////////////////////////////////////
// plx::FilesInfo
//
namespace plx {
#pragma comment(user, "plex.define=plex_vista_support")
class FilesInfo {
private:
  FILE_ID_BOTH_DIR_INFO* info_;
  plx::LinkedBuffers link_buffs_;
  mutable bool done_;

public:
  static FilesInfo FromDir(plx::File& file, long buffer_hint = 32) {
    if (file.status() != (plx::File::directory | plx::File::existing))
      throw plx::IOException(__LINE__, nullptr);

    FilesInfo finf;
    // |buffer_size| controls the tradeoff between speed and memory use.
    const size_t buffer_size = buffer_hint * 128;

    plx::Range<unsigned char> data;
    for (size_t count = 0; ;++count) {
      data = finf.link_buffs_.new_buffer(buffer_size);
      if (!::GetFileInformationByHandleEx(
          file.handle_,
          count == 0 ? FileIdBothDirectoryRestartInfo: FileIdBothDirectoryInfo,
          data.start(), plx::To<DWORD>(data.size()))) {
        if (::GetLastError() != ERROR_NO_MORE_FILES)
          throw plx::IOException(__LINE__, nullptr);
        break;
      }
    }
    // The last buffer contains garbage. remove it.
    finf.link_buffs_.remove_last_buffer();
    return std::move(finf);
  }

  void first() {
    done_ = false;
    link_buffs_.first();
    info_ = reinterpret_cast<FILE_ID_BOTH_DIR_INFO*>(link_buffs_.get().start());
  }

  void next() {
    if (!info_->NextEntryOffset) {
      // last entry of this buffer. Move to next buffer.
      link_buffs_.next();
      if (link_buffs_.done()) {
        done_ = true;
      } else {
        info_ = reinterpret_cast<FILE_ID_BOTH_DIR_INFO*>(link_buffs_.get().start());
      }
    } else {
      info_ =reinterpret_cast<FILE_ID_BOTH_DIR_INFO*>(
          ULONG_PTR(info_) + info_->NextEntryOffset);
    }
  }

  bool done() const {
    return done_;
  }

  const plx::ItRange<wchar_t*> file_name() const {
    return plx::ItRange<wchar_t*>(info_->FileName, info_->FileName+ info_->FileNameLength);
  }

  FilesInfo(FilesInfo&& other)
      : link_buffs_(std::move(other.link_buffs_)) {
    std::swap(info_, other.info_);
    std::swap(done_, other.done_);
  }

private:
  FilesInfo() : info_(nullptr), done_(false) {
  }

  FilesInfo(const FilesInfo&);
};

}
