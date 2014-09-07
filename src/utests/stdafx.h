// This is the plex precompiled header, not the
// same as the VC precompiled header.

#pragma once
#define NOMINMAX

#include <windows.h>






#include <intrin.h>
#include <stdlib.h>
#include <nmmintrin.h>
#include <string.h>
#include <array>
#include <functional>
#include <initializer_list>
#include <cctype>
#include <iterator>
#include <list>
#include <map>
#include <algorithm>
#include <utility>
#include <limits>
#include <type_traits>
#include <string>
#include <memory>
#include <vector>
#include <stdint.h>
#include <stdarg.h>

const int plex_sse42_support = 1;

const int plex_cpuid_support = 1;

const int plex_vista_support = 1;


///////////////////////////////////////////////////////////////////////////////
// plx::CRC32C (computes CRC-32 checksum, SSE4 accelerated)
// Polinomal 0x1EDC6F41 aka iSCSI CRC. This gives a 10^-41 probabilty of not
// detecting a 3-bit burst error and 10^-40 for sporadic one bit errors.
//
namespace plx {
#pragma comment(user, "plex.define=plex_sse42_support")

uint32_t CRC32C(uint32_t crc, const char *buf, size_t len) ;


///////////////////////////////////////////////////////////////////////////////
// plx::Exception
// line_ : The line of code, usually __LINE__.
// message_ : Whatever useful text.
//
class Exception {
  int line_;
  const char* message_;

protected:
  void PostCtor() {
    if (::IsDebuggerPresent()) {
      //__debugbreak();
    }
  }

public:
  Exception(int line, const char* message) : line_(line), message_(message) {}
  virtual ~Exception() {}
  const char* Message() const { return message_; }
  int Line() const { return line_; }
};


///////////////////////////////////////////////////////////////////////////////
// plx::CpuId
// id_ : the four integers returned by the 'cpuid' instruction.
#pragma comment(user, "plex.define=plex_cpuid_support")

class CpuId {
  int id_[4];

 public:
  CpuId() {
    __cpuid(id_, 1);
  }

  int stepping() const { return id_[0] & 0x0f; }
  int model() const { return (id_[0] >> 4) & 0x0f; }
  int family() const { return (id_[0] >> 8) & 0x0f; }
  int type() const { return (id_[0] >> 12) & 0x03; }
  int logical_procesors() const { return (id_[1] >> 16) & 0xff; }

  bool sse3() const { return (id_[2] & (1 << 0)) != 0; }
  bool pclmuldq() const { return (id_[2] & (1 << 1)) != 0; }
  bool dtes64() const { return (id_[2] & (1 << 2)) != 0; }
  bool monitor() const { return (id_[2] & (1 << 3)) != 0; }
  bool dscpl() const { return (id_[2] & (1 << 4)) != 0; }
  bool vmx() const { return (id_[2] & (1 << 5)) != 0; }
  bool smx() const { return (id_[2] & (1 << 6)) != 0; }
  bool eist() const { return (id_[2] & (1 << 7)) != 0; }
  bool tm2() const { return (id_[2] & (1 << 8)) != 0; }
  bool ssse3() const { return (id_[2] & (1 << 9)) != 0; }
  bool cnxtid() const { return (id_[2] & (1 << 10)) != 0; }

  bool fma() const { return (id_[2] & (1 << 12)) != 0; }
  bool cx16() const { return (id_[2] & (1 << 13)) != 0; }
  bool xtpr() const { return (id_[2] & (1 << 14)) != 0; }
  bool pdcm() const { return (id_[2] & (1 << 15)) != 0; }

  bool pcid() const { return (id_[2] & (1 << 17)) != 0; }
  bool dca() const { return (id_[2] & (1 << 18)) != 0; }
  bool sse41() const { return (id_[2] & (1 << 19)) != 0; }
  bool sse42() const { return (id_[2] & (1 << 20)) != 0; }
  bool x2apic() const { return (id_[2] & (1 << 21)) != 0; }
  bool movbe() const { return (id_[2] & (1 << 22)) != 0; }
  bool popcnt() const { return (id_[2] & (1 << 23)) != 0; }
  bool tscdeadline() const { return (id_[2] & (1 << 24)) != 0; }
  bool aes() const { return (id_[2] & (1 << 25)) != 0; }
  bool xsave() const { return (id_[2] & (1 << 26)) != 0; }
  bool osxsave() const { return (id_[2] & (1 << 27)) != 0; }
  bool avx() const { return (id_[2] & (1 << 28)) != 0; }
  bool f16c() const { return (id_[2] & (1 << 29)) != 0; }
  bool rdrand() const { return (id_[2] & (1 << 30)) != 0; }

  bool fpu() const { return (id_[3] & (1 << 0)) != 0; }
  bool vme() const { return (id_[3] & (1 << 1)) != 0; }
  bool de() const { return (id_[3] & (1 << 2)) != 0; }
  bool pse() const { return (id_[3] & (1 << 3)) != 0; }
  bool tsc() const { return (id_[3] & (1 << 4)) != 0; }
  bool msr() const { return (id_[3] & (1 << 5)) != 0; }
  bool pae() const { return (id_[3] & (1 << 6)) != 0; }
  bool mce() const { return (id_[3] & (1 << 7)) != 0; }
  bool cx8() const { return (id_[3] & (1 << 8)) != 0; }
  bool apic() const { return (id_[3] & (1 << 9)) != 0; }

  bool sep() const { return (id_[3] & (1 << 11)) != 0; }
  bool mtrr() const { return (id_[3] & (1 << 12)) != 0; }
  bool pge() const { return (id_[3] & (1 << 13)) != 0; }
  bool mca() const { return (id_[3] & (1 << 14)) != 0; }
  bool cmov() const { return (id_[3] & (1 << 15)) != 0; }
  bool pat() const { return (id_[3] & (1 << 16)) != 0; }
  bool pse36() const { return (id_[3] & (1 << 17)) != 0; }
  bool psn() const { return (id_[3] & (1 << 18)) != 0; }
  bool clfsh() const { return (id_[3] & (1 << 19)) != 0; }

  bool ds() const { return (id_[3] & (1 << 21)) != 0; }
  bool acpi() const { return (id_[3] & (1 << 22)) != 0; }
  bool mmx() const { return (id_[3] & (1 << 23)) != 0; }
  bool fxsr() const { return (id_[3] & (1 << 24)) != 0; }
  bool sse() const { return (id_[3] & (1 << 25)) != 0; }
  bool sse2() const { return (id_[3] & (1 << 26)) != 0; }
  bool ss() const { return (id_[3] & (1 << 27)) != 0; }
  bool htt() const { return (id_[3] & (1 << 28)) != 0; }
  bool tm() const { return (id_[3] & (1 << 29)) != 0; }

  bool pbe() const { return (id_[3] & (1 << 31)) != 0; }
};


///////////////////////////////////////////////////////////////////////////////
// plx::RangeException (thrown by ItRange and others)
//
class RangeException : public plx::Exception {
  void* ptr_;
public:
  RangeException(int line, void* ptr)
      : Exception(line, "Invalid Range"), ptr_(ptr) {
    PostCtor();
  }

  void* pointer() const {
    return ptr_;
  }
};



///////////////////////////////////////////////////////////////////////////////
// plx::ItRange
// s_ : first element
// e_ : one past the last element
//
template <typename It>
class ItRange {
  It s_;
  It e_;

public:
  typedef typename std::iterator_traits<
      typename std::remove_reference<It>::type
  >::reference RefT;

  typedef typename std::iterator_traits<
      typename std::remove_reference<It>::type
  >::value_type ValueT;

  typedef typename std::remove_const<It>::type NoConstIt;

  ItRange() : s_(), e_() {
  }

  template <typename U>
  ItRange(const ItRange<U>& other) : s_(other.start()), e_(other.end()) {
  }

  ItRange(It start, It end) : s_(start), e_(end) {
  }

  ItRange(It start, size_t size) : s_(start), e_(start + size) {
  }

  bool empty() const {
    return (s_ == e_);
  }

  size_t size() const {
    return (e_ - s_);
  }

  It start() const {
    return s_;
  }

  It end() const {
    return e_;
  }

  bool valid() const {
    return (e_ >= s_);
  }

  RefT front() const {
    if (s_ >= e_)
      throw plx::RangeException(__LINE__, nullptr);
    return s_[0];
  }

  RefT back() const {
    return e_[-1];
  }

  RefT operator[](size_t i) const {
    return s_[i];
  }

  bool equals(const ItRange<It>& o) const {
    if (o.size() != size())
      return false;
    return (memcmp(s_, o.s_, size()) == 0);
  }

  size_t starts_with(const ItRange<It>& o) const {
    if (o.size() > size())
      return 0;
    return (memcmp(s_, o.s_, o.size()) == 0) ? o.size() : 0;
  }

  bool contains(ValueT x, size_t* pos) const {
    auto c = s_;
    while (c != e_) {
      if (*c == x) {
        *pos = c - s_;
        return true;
      }
      ++c;
    }
    return false;
  }

  template <size_t count>
  size_t CopyToArray(ValueT (&arr)[count]) const {
    auto copied = std::min(size(), count);
    auto last = copied + s_;
    std::copy(s_, last, arr);
    return copied;
  }

  template <size_t count>
  size_t CopyToArray(std::array<ValueT, count>& arr) const {
    auto copied = std::min(size(), count);
    auto last = copied + s_;
    std::copy(s_, last, arr.begin());
    return copied;
  }

  intptr_t advance(size_t count) {
    auto ns = s_ + count;
    if (ns > e_)
      return (e_ - ns);
    s_ = ns;
    return size();
  }

  void clear() {
    s_ = It();
    e_ = It();
  }

  void reset_start(It new_start) {
    auto sz = size();
    s_ = new_start;
    e_ = s_ + sz;
  }

  ItRange<const unsigned char*> const_bytes() const {
    auto s = reinterpret_cast<const unsigned char*>(s_);
    auto e = reinterpret_cast<const unsigned char*>(e_);
    return ItRange<const unsigned char*>(s, e);
  }

   ItRange<unsigned char*> bytes() const {
    auto s = reinterpret_cast<unsigned char*>(s_);
    auto e = reinterpret_cast<unsigned char*>(e_);
    return ItRange<unsigned char*>(s, e);
  }

};

template <typename U, size_t count>
ItRange<U*> RangeFromLitStr(U (&str)[count]) {
  return ItRange<U*>(str, str + count - 1);
}

template <typename U, size_t count>
ItRange<U*> RangeFromArray(U (&str)[count]) {
  return ItRange<U*>(str, str + count);
}

template <typename U>
ItRange<U*> RangeUntilValue(U* start, U value) {
  auto stop = start;
  while (*stop != value) {
    ++stop;
  }
  return ItRange<U*>(start, stop);
}



ItRange<uint8_t*> RangeFromBytes(void* start, size_t count) ;

ItRange<const uint8_t*> RangeFromBytes(const void* start, size_t count) ;

template <typename U>
std::string StringFromRange(const ItRange<U>& r) {
  return std::string(r.start(), r.end());
}

template <typename U>
std::wstring WideStringFromRange(const ItRange<U>& r) {
  return std::wstring(r.start(), r.end());
}

template <typename U>
std::unique_ptr<U[]> HeapRange(ItRange<U*>&r) {
  std::unique_ptr<U[]> ptr(new U[r.size()]);
  r.reset_start(ptr.get());
  return ptr;
}


///////////////////////////////////////////////////////////////////////////////
// plx::Range  (alias for ItRange<T*>)
//
template <typename T>
using Range = plx::ItRange<T*>;


///////////////////////////////////////////////////////////////////////////////
// plx::IOException
// error_code_ : The win32 error code of the last operation.
// name_ : The file or pipe in question.
//
class IOException : public plx::Exception {
  DWORD error_code_;
  const std::wstring name_;

public:
  IOException(int line, const wchar_t* name)
      : Exception(line, "IO problem"),
        error_code_(::GetLastError()),
        name_(name) {
    PostCtor();
  }
  DWORD ErrorCode() const { return error_code_; }
  const wchar_t* Name() const { return name_.c_str(); }
};


///////////////////////////////////////////////////////////////////////////////
// plx::InvalidParamException
// parameter_ : the position of the offending parameter, zero if unknown.
//
class InvalidParamException : public plx::Exception {
  int parameter_;

public:
  InvalidParamException(int line, int parameter)
      : Exception(line, "Invalid parameter"), parameter_(parameter) {
    PostCtor();
  }
  int Parameter() const { return parameter_; }
};


///////////////////////////////////////////////////////////////////////////////
// plx::FilePath
// path_ : The actual path (ucs16 probably).
//
class FilePath {
private:
  std::wstring path_;
  friend class File;

public:
  explicit FilePath(const wchar_t* path)
    : path_(path) {
  }

  explicit FilePath(const std::wstring& path)
    : path_(path) {
  }

  FilePath parent() const {
    auto pos = path_.find_last_of(L'\\');
    if (pos == std::string::npos)
      return FilePath();
    return FilePath(path_.substr(0, pos));
  }

  std::wstring leaf() const {
    auto pos = path_.find_last_of(L'\\');
    if (pos == std::string::npos) {
      return is_drive() ? std::wstring() : path_;
    }
    return path_.substr(pos + 1);
  }

  FilePath append(const std::wstring& name) const {
    if (name.empty())
      throw plx::IOException(__LINE__, path_.c_str());

    std::wstring full(path_);
    if (!path_.empty())
      full.append(1, L'\\');
    full.append(name);
    return FilePath(full);
  }

  bool is_drive() const {
    return (path_.size() != 2) ? false : drive_impl();
  }

  bool has_drive() const {
    return (path_.size() < 2) ? false : drive_impl();
  }

  const wchar_t* raw() const {
    return path_.c_str();
  }

private:
  FilePath() {}

  bool drive_impl() const {
    if (path_[1] != L':')
      return false;
    auto dl = path_[0];
    if ((dl >= L'A') && (dl <= L'Z'))
      return true;
    if ((dl >= L'a') && (dl <= L'z'))
      return true;
    return false;
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::FileSecurity
//

///////////////////////////////////////////////////////////////////////////////
//
class FileSecurity {
private:
  SECURITY_ATTRIBUTES* sattr_;
  friend class File;
public:
  FileSecurity()
    : sattr_(nullptr) {
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::LinkedBuffers
//
class LinkedBuffers {
  struct Item {
    size_t size;
    std::unique_ptr<unsigned char[]> data;
    Item(size_t size)
        : size(size), data(new unsigned char[size]) {
    }

    Item(const Item& other)
        : size(other.size), data(new unsigned char[size]) {
      memcpy(data.get(), other.data.get(), size);
    }

    Item(Item&& other) = delete;
  };

  typedef std::list<Item> BList;

  BList buffers_;
  BList::iterator loop_it_;

public:
  LinkedBuffers() {
  }

  LinkedBuffers(const LinkedBuffers& other)
      : buffers_(other.buffers_) {
  }

  LinkedBuffers(LinkedBuffers&& other) {
    buffers_.swap(other.buffers_);
    std::swap(loop_it_, other.loop_it_);
  }

  plx::Range<unsigned char> new_buffer(size_t size_bytes) {
    buffers_.emplace_back(size_bytes);
    auto start = &(buffers_.back().data)[0];
    return plx::Range<unsigned char>(start, start + size_bytes);
  }

  void remove_last_buffer() {
    buffers_.pop_back();
  }

  void first() {
    loop_it_ = begin(buffers_);
  }

  void next() {
    ++loop_it_;
  }

  bool done() {
    return (loop_it_ == end(buffers_));
  }

  plx::Range<unsigned char> get() {
    auto start = &(loop_it_->data)[0];
    return plx::Range<unsigned char>(start, start + loop_it_->size);
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::GetExePath
//
plx::FilePath GetExePath() ;


///////////////////////////////////////////////////////////////////////////////
// plx::Hash_FNV1a_32  (nice hash function for strings used by c++ std)
// for short inputs is about 100 times faster than SHA1 and about 20 times
// faster for long inputs.
uint32_t Hash_FNV1a_32(const plx::Range<const unsigned char>& r) ;


///////////////////////////////////////////////////////////////////////////////
// plx::Hash_FNV1a_64  (nice hash function for strings used by c++ std)
// for short inputs is about 100 times faster than SHA1 and about 20 times
// faster for long inputs.
uint64_t Hash_FNV1a_64(const plx::Range<const unsigned char>& r) ;



///////////////////////////////////////////////////////////////////////////////
// HexASCII (converts a byte into a two-char readable representation.
//
static const char HexASCIITable[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

char* HexASCII(uint8_t byte, char* out) ;


///////////////////////////////////////////////////////////////////////////////
std::string HexASCIIStr(const plx::Range<const uint8_t>& r, char separator) ;



///////////////////////////////////////////////////////////////////////////////
// plx::IOCPLoop  (message loop basd on IO completion ports)
// plx::IOCPProxy (the way to post tasks to the  the IOCPLoop)
//

class IOCPLoopProxy {
  HANDLE port_;
  friend class IOCPLoop;

  IOCPLoopProxy(HANDLE port) : port_(port) {
  }

public:
  bool PostTask(const std::function<void()>& closure) {
    auto cc = new std::function<void()>(closure);
    if (!::PostQueuedCompletionStatus(port_, 0, ULONG_PTR(cc), NULL)) {
      delete cc;
      return false;
    }
    return true;
  }

  bool PostQuitTask() {
    return TRUE == ::PostQueuedCompletionStatus(port_, 0, 0, NULL);
  }
};

class IOCPLoop {
  HANDLE port_;
  unsigned int calls_;

public:
  IOCPLoop() : port_(nullptr), calls_(0) {
    port_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
  }

  IOCPLoop(const IOCPLoop&) = delete;

  ~IOCPLoop() {
    ::CloseHandle(port_);
  }

  IOCPLoopProxy MakeProxy() {
    return IOCPLoopProxy(port_);
  }

  unsigned int call_count() const {
    return calls_;
  }

  void Run(unsigned long timeout_ms) {
    DWORD bytes = 0;
    OVERLAPPED* ov;
    ULONG_PTR key;
    while (true) {
      ::GetQueuedCompletionStatus(port_, &bytes, &key, &ov, timeout_ms);
      if (!key)
        break;
      auto closure = reinterpret_cast<std::function<void ()>*>(key);
      (*closure)();
      ++calls_;
      delete closure;
    }
  }

};


///////////////////////////////////////////////////////////////////////////////
// plx::FileParams
// access_  : operations allowed to the file.
// sharing_ : operations others can do to the file.
// disposition_ : if the file is reset or opened as is.
// flags_ : hints or special modes go here.
//
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

  static FileParams Directory_ShareAll() {
    return FileParams(FILE_GENERIC_READ, kShareAll,
                     OPEN_EXISTING,
                     0, FILE_FLAG_BACKUP_SEMANTICS, 0);
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::File
//
class File {
  HANDLE handle_;
  unsigned int  status_;
  friend class FilesInfo;

private:
  File(HANDLE handle,
       unsigned int status)
    : handle_(handle),
      status_(status) {
  }

  File();
  File(const File&);
  File& operator=(const File&);

public:
  enum Status {
    none              = 0,
    brandnew          = 1,   //
    existing          = 2,
    delete_on_close   = 4,
    directory         = 8,   // $$$ just use backup semantics?
    exclusive         = 16,
    readonly          = 32,
    information       = 64,
  };

  File(File&& file)
    : handle_(INVALID_HANDLE_VALUE),
      status_(none) {
    std::swap(file.handle_, handle_);
    std::swap(file.status_, status_);
  }

  static File Create(const plx::FilePath& path,
                     const plx::FileParams& params,
                     const plx::FileSecurity& security) {
    HANDLE file = ::CreateFileW(path.path_.c_str(),
                                params.access_,
                                params.sharing_,
                                security.sattr_,
                                params.disposition_,
                                params.attributes_ | params.flags_ | params.sqos_,
                                0);
    DWORD gle = ::GetLastError();
    unsigned int status = none;

    if (file != INVALID_HANDLE_VALUE) {

      switch (params.disposition_) {
        case OPEN_EXISTING:
        case TRUNCATE_EXISTING:
          status |= existing; break;
        case CREATE_NEW:
          status |= (gle == ERROR_FILE_EXISTS) ? existing : brandnew; break;
        case CREATE_ALWAYS:
        case OPEN_ALWAYS:
          status |= (gle == ERROR_ALREADY_EXISTS) ? existing : brandnew; break;
        default:
          __debugbreak();
      };

      if (params.flags_ == FILE_FLAG_BACKUP_SEMANTICS) status |= directory;
      if (params.flags_ & FILE_FLAG_DELETE_ON_CLOSE) status |= delete_on_close;
      if (params.sharing_ == 0) status |= exclusive;
      if (params.access_ == 0) status |= information;
      if (params.access_ == GENERIC_READ) status |= readonly;
    }

    return File(file, status);
  }

  ~File() {
    if (handle_ != INVALID_HANDLE_VALUE) {
      if (!::CloseHandle(handle_)) {
        throw IOException(__LINE__, nullptr);
      }
    }
  }

  // $$ this ignores the volume id, so technically incorrect.
  long long get_unique_id() {
    BY_HANDLE_FILE_INFORMATION bhfi;
    if (!::GetFileInformationByHandle(handle_, &bhfi))
      throw IOException(__LINE__, nullptr);
    LARGE_INTEGER li = { bhfi.nFileIndexLow, bhfi.nFileIndexHigh };
    return li.QuadPart;
  }

  unsigned int status() const {
    return status_;
  }

  bool is_valid() const {
    return (handle_ != INVALID_HANDLE_VALUE);
  }

  long long size_in_bytes() const {
    LARGE_INTEGER li = {0};
    ::GetFileSizeEx(handle_, &li);
    return li.QuadPart;
  }

  size_t read(plx::Range<char>& mem, unsigned int from) {
    OVERLAPPED ov = {0};
    ov.Offset = from;
    DWORD read = 0;
    if (!::ReadFile(handle_, mem.start(), static_cast<DWORD>(mem.size()),
                    &read, &ov))
      return 0;
    return read;
  }

  size_t write(const plx::Range<const char>& mem, int from = -1) {
    return write(mem.start(), mem.size(), from);
  }

  size_t write(const plx::Range<char>& mem, int from = -1) {
    return write(mem.start(), mem.size(), from);
  }

  size_t write(const char* buf, size_t len, int from) {
    OVERLAPPED ov = {0};
    ov.Offset = from;
    DWORD written = 0;
    if (!::WriteFile(handle_, buf, static_cast<DWORD>(len),
                     &written, (from < 0) ? nullptr : &ov))
      return 0;
    return written;
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::CodecException (thrown by some decoders)
// bytes_ : The 16 bytes or less that caused the issue.
//
class CodecException : public plx::Exception {
  uint8_t bytes_[16];
  size_t count_;

public:
  CodecException(int line, const plx::Range<const unsigned char>* br)
      : Exception(line, "Codec exception"), count_(0) {
    if (br)
      count_ = br->CopyToArray(bytes_);
    PostCtor();
  }

  std::string bytes() const {
    return plx::HexASCIIStr(plx::Range<const uint8_t>(bytes_, count_), ',');
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::JsonType
//
enum class JsonType {
  NULLT,
  BOOL,
  INT64,
  DOUBLE,
  ARRAY,
  OBJECT,
  STRING,
};


///////////////////////////////////////////////////////////////////////////////
// plx::JsonValue
// type_ : the actual type from the Data union.
// u_ : the storage for all the possible values.
template <typename T> using AligedStore =
    std::aligned_storage<sizeof(T), __alignof(T)>;

class JsonValue {
  //typedef std::unordered_map<std::string, JsonValue> ObjectImpl;
  typedef std::map<std::string, JsonValue> ObjectImpl;
  typedef std::vector<JsonValue> ArrayImpl;
  typedef std::string StringImpl;

  plx::JsonType type_;
  union Data {
    bool bolv;
    double dblv;
    int64_t intv;
    AligedStore<StringImpl>::type str;
    AligedStore<ArrayImpl>::type arr;
    AligedStore<ObjectImpl>::type obj;
  } u_;

 public:

  JsonValue() : type_(JsonType::NULLT) {
  }

  JsonValue(const plx::JsonType& type) : type_(type) {
    if (type_ == JsonType::ARRAY)
      new (&u_.arr) ArrayImpl;
    else if (type_ == JsonType::OBJECT)
      new (&u_.obj) ObjectImpl();
    else
      throw plx::InvalidParamException(__LINE__, 1);
  }

  JsonValue(const JsonValue& other) : type_(JsonType::NULLT) {
    *this = other;
  }

  JsonValue(JsonValue&& other) : type_(JsonType::NULLT) {
    *this = std::move(other);
  }

  ~JsonValue() {
    Destroy();
  }

  JsonValue(nullptr_t) : type_(JsonType::NULLT) {
  }

  JsonValue(bool b) : type_(JsonType::BOOL) {
    u_.bolv = b;
  }

  JsonValue(int v) : type_(JsonType::INT64) {
    u_.intv = v;
  }

  JsonValue(int64_t v) : type_(JsonType::INT64) {
    u_.intv = v;
  }

  JsonValue(double v) : type_(JsonType::DOUBLE) {
    u_.dblv = v;
  }

  JsonValue(const std::string& s) : type_(JsonType::STRING) {
    new (&u_.str) std::string(s);
  }

  JsonValue(const char* s) : type_(JsonType::STRING) {
    new (&u_.str) StringImpl(s);
  }

  JsonValue(std::initializer_list<JsonValue> il) : type_(JsonType::ARRAY) {
    new (&u_.arr) ArrayImpl(il.begin(), il.end());
  }

  template<class It>
  JsonValue(It first, It last) : type_(JsonType::ARRAY) {
    new (&u_.arr) ArrayImpl(first, last);
  }

  JsonValue& operator=(const JsonValue& other) {
    if (this != &other) {
      Destroy();

      if (other.type_ == JsonType::BOOL)
        u_.bolv = other.u_.bolv;
      else if (other.type_ == JsonType::INT64)
        u_.intv = other.u_.intv;
      else if (other.type_ == JsonType::DOUBLE)
        u_.dblv = other.u_.dblv;
      else if (other.type_ == JsonType::ARRAY)
        new (&u_.arr) ArrayImpl(*other.GetArray());
      else if (other.type_ == JsonType::OBJECT)
        new (&u_.obj) ObjectImpl(*other.GetObject());
      else if (other.type_ == JsonType::STRING)
        new (&u_.str) StringImpl(*other.GetString());

      type_ = other.type_;
    }
    return *this;
  }

  JsonValue& operator=(JsonValue&& other) {
    if (this != &other) {
      Destroy();

      if (other.type_ == JsonType::BOOL)
        u_.bolv = other.u_.bolv;
      else if (other.type_ == JsonType::INT64)
        u_.intv = other.u_.intv;
      else if (other.type_ == JsonType::DOUBLE)
        u_.dblv = other.u_.dblv;
      else if (other.type_ == JsonType::ARRAY)
        new (&u_.arr) ArrayImpl(std::move(*other.GetArray()));
      else if (other.type_ == JsonType::OBJECT)
        new (&u_.obj) ObjectImpl(std::move(*other.GetObject()));
      else if (other.type_ == JsonType::STRING)
        new (&u_.str) StringImpl(std::move(*other.GetString()));

      type_ = other.type_;
    }
    return *this;
  }

  JsonValue& operator[](const std::string& s) {
    return (*GetObject())[s];
  }

  JsonValue& operator[](size_t ix) {
    return (*GetArray())[ix];
  }

  plx::JsonType type() const {
    return type_;
  }

  bool get_bool() const {
    return u_.bolv;
  }

  int64_t get_int64() const {
    return u_.intv;
  }

  double get_double() const {
    return u_.dblv;
  }

  std::string get_string() const {
    return *GetString();
  }

  bool has_key(const std::string& k) {
    return (GetObject()->find(k) != end(*GetObject()));
  }

  void push_back(JsonValue&& value) {
    GetArray()->push_back(value);
  }

  size_t size() const {
   if (type_ == JsonType::ARRAY)
      return GetArray()->size();
    else if (type_ == JsonType::OBJECT)
      return GetObject()->size();
    else return 0;
  }

 private:

  ObjectImpl* GetObject() {
    if (type_ != JsonType::OBJECT)
      throw 1;
    void* addr = &u_.obj;
    return reinterpret_cast<ObjectImpl*>(addr);
  }

  const ObjectImpl* GetObject() const {
    if (type_ != JsonType::OBJECT)
      throw 1;
    const void* addr = &u_.obj;
    return reinterpret_cast<const ObjectImpl*>(addr);
  }

  ArrayImpl* GetArray() {
    if (type_ != JsonType::ARRAY)
      throw 1;
    void* addr = &u_.arr;
    return reinterpret_cast<ArrayImpl*>(addr);
  }

  const ArrayImpl* GetArray() const {
    if (type_ != JsonType::ARRAY)
      throw 1;
    const void* addr = &u_.arr;
    return reinterpret_cast<const ArrayImpl*>(addr);
  }

  std::string* GetString() {
    if (type_ != JsonType::STRING)
      throw 1;
    void* addr = &u_.str;
    return reinterpret_cast<StringImpl*>(addr);
  }

  const std::string* GetString() const {
    if (type_ != JsonType::STRING)
      throw 1;
    const void* addr = &u_.str;
    return reinterpret_cast<const StringImpl*>(addr);
  }

  void Destroy() {
    if (type_ == JsonType::ARRAY)
      GetArray()->~ArrayImpl();
    else if (type_ == JsonType::OBJECT)
      GetObject()->~ObjectImpl();
    else if (type_ == JsonType::STRING)
      GetString()->~StringImpl();
  }

};


///////////////////////////////////////////////////////////////////////////////
// plx::OverflowKind
//
enum class OverflowKind {
  None,
  Positive,
  Negative,
};


///////////////////////////////////////////////////////////////////////////////
// plx::LocalUniqueId.
// requres advapi32.lib
//
uint64_t LocalUniqueId() ;


///////////////////////////////////////////////////////////////////////////////
// plx::ScopeGuardBase
// dismissed_ : wether or not function_ will be called.
// function_  : the cleanup function (user defined).
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


///////////////////////////////////////////////////////////////////////////////
// plx::OverflowException (thrown by some numeric converters)
// kind_ : Type of overflow, positive or negative.
//
class OverflowException : public plx::Exception {
  plx::OverflowKind kind_;

public:
  OverflowException(int line, plx::OverflowKind kind)
      : Exception(line, "Overflow"), kind_(kind) {
    PostCtor();
  }
  plx::OverflowKind kind() const { return kind_; }
};


///////////////////////////////////////////////////////////////////////////////
// plx::NextInt  integer promotion.

short NextInt(char value) ;

int NextInt(short value) ;

long long NextInt(int value) ;

long long NextInt(long value) ;

long long NextInt(long long value) ;

short NextInt(unsigned char value) ;

int NextInt(unsigned short value) ;

long long NextInt(unsigned int value) ;

long long NextInt(unsigned long value) ;

long long NextInt(unsigned long long value) ;


///////////////////////////////////////////////////////////////////////////////
// plx::To  (integer to integer type safe cast)
//

template <bool src_signed, bool tgt_signed>
struct ToCastHelper;

template <>
struct ToCastHelper<false, false> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (sizeof(Tgt) >= sizeof(Src)) {
      return static_cast<Tgt>(value);
    } else {
      if (value > std::numeric_limits<Tgt>::max())
        throw plx::OverflowException(__LINE__, OverflowKind::Positive);
      if (value < std::numeric_limits<Tgt>::min())
        throw plx::OverflowException(__LINE__, OverflowKind::Negative);
      return static_cast<Tgt>(value);
    }
  }
};

template <>
struct ToCastHelper<true, true> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (sizeof(Tgt) >= sizeof(Src)) {
      return static_cast<Tgt>(value);
    } else {
      if (value > std::numeric_limits<Tgt>::max())
        throw plx::OverflowException(__LINE__, OverflowKind::Positive);
      if (value < std::numeric_limits<Tgt>::min())
        throw plx::OverflowException(__LINE__, OverflowKind::Negative);
      return static_cast<Tgt>(value);
    }
  }
};

template <>
struct ToCastHelper<false, true> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (plx::NextInt(value) > std::numeric_limits<Tgt>::max())
      throw plx::OverflowException(__LINE__, OverflowKind::Positive);
    if (plx::NextInt(value) < std::numeric_limits<Tgt>::min())
      throw plx::OverflowException(__LINE__, OverflowKind::Negative);
    return static_cast<Tgt>(value);
  }
};

template <>
struct ToCastHelper<true, false> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (value < Src(0))
      throw plx::OverflowException(__LINE__, OverflowKind::Negative);
    if (unsigned(value) > std::numeric_limits<Tgt>::max())
      throw plx::OverflowException(__LINE__, OverflowKind::Positive);
    return static_cast<Tgt>(value);
  }
};

template <typename Tgt, typename Src>
typename std::enable_if<
    std::numeric_limits<Tgt>::is_integer &&
    std::numeric_limits<Src>::is_integer,
    Tgt>::type
To(const Src & value) {
  return ToCastHelper<std::numeric_limits<Src>::is_signed,
                      std::numeric_limits<Tgt>::is_signed>::cast<Tgt>(value);
}


///////////////////////////////////////////////////////////////////////////////
// SkipWhitespace (advances a range as long isspace() is false.
//
template <typename T>
typename std::enable_if<
    sizeof(T) == 1,
    plx::Range<T>>::type
SkipWhitespace(const plx::Range<T>& r) {
  auto wr = r;
  while (!wr.empty()) {
    if (!std::isspace(wr.front()))
      break;
    wr.advance(1);
  }
  return wr;
}


///////////////////////////////////////////////////////////////////////////////
// plx::PlatformCheck : checks if the code can be run in this setting.
//
bool PlatformCheck() ;;


///////////////////////////////////////////////////////////////////////////////
// plx::DecodeString (decodes a json-style encoded string)
//
std::string DecodeString(plx::Range<const char>& range) ;


///////////////////////////////////////////////////////////////////////////////
// plx::DecodeUTF8 (decodes a UTF8 codepoint into a 32-bit codepoint)
//
// bits encoding
// 7    0xxxxxxx
// 11   110xxxxx 10xxxxxx
// 16   1110xxxx 10xxxxxx 10xxxxxx
// 21   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// The 5 and 6 bytes encoding are no longer valid since RFC 3629.
// max point is then U+10FFFF.
//
static const uint32_t Utf8BitMask[] = {
  (1 << 7) - 1,   // 0000 0000 0000 0000 0111 1111
  (1 << 11) - 1,  // 0000 0000 0000 0111 1111 1111
  (1 << 16) - 1,  // 0000 0000 1111 1111 1111 1111
  (1 << 21) - 1   // 0001 1111 1111 1111 1111 1111
};

char32_t DecodeUTF8(plx::Range<const unsigned char>& ir) ;


///////////////////////////////////////////////////////////////////////////////
// plx::ParseJsonValue (converts a JSON string into a JsonValue)
//
plx::JsonValue ParseJsonValue(plx::Range<const char>& range);

plx::JsonValue ParseJsonValue(plx::Range<const char>& range) ;


///////////////////////////////////////////////////////////////////////////////
// plx::StringPrintf  (c-style printf for std strings)
//

std::string StringPrintf(const char* fmt, ...) ;


///////////////////////////////////////////////////////////////////////////////
// plx::FilesInfo
//
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

  FilesInfo(FilesInfo&& other)
      : link_buffs_(std::move(other.link_buffs_)) {
    std::swap(info_, other.info_);
    std::swap(done_, other.done_);
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
    return plx::ItRange<wchar_t*>(
      info_->FileName,
      info_->FileName+ (info_->FileNameLength / sizeof(wchar_t)));
  }

  long long creation_ns1600() const {
    return info_->CreationTime.QuadPart;
  }

  bool is_directory() const {
    return info_->FileAttributes & FILE_ATTRIBUTE_DIRECTORY? true : false;
  }

private:
  FilesInfo() : info_(nullptr), done_(false) {
  }

  FilesInfo(const FilesInfo&);
};

}
