// ut_core.cpp
#define NOMINMAX

#include "../utests.h"




#include <windows.h>
#include <intrin.h>
#include <nmmintrin.h>
#include <string.h>
#include <array>
#include <functional>
#include <initializer_list>
#include <cctype>
#include <iterator>
#include <map>
#include <algorithm>
#include <utility>
#include <limits>
#include <type_traits>
#include <string>
#include <vector>
#include <stdint.h>
const int plex_sse42_support = 1;
const int plex_cpuid_support = 1;

///////////////////////////////////////////////////////////////////////////////
// plx::CRC32C (computes CRC-32 checksum, SSE4 accelerated)
// Polinomal 0x1EDC6F41 aka iSCSI CRC. This gives a 10^-41 probabilty of not
// detecting a 3-bit burst error and 10^-40 for sporadic one bit errors.
//
namespace plx {
#pragma comment(user, "plex.define=plex_sse42_support")

uint32_t CRC32C(uint32_t crc, const char *buf, size_t len) {
  if (len == 0)
    return crc;
  crc = ~crc;
  // Process one byte at a time until aligned.
  for (; (len > 0) && (reinterpret_cast<uintptr_t>(buf) & 0x03UL); len--, buf++) {
    crc = _mm_crc32_u8(crc, *buf);
  }
  // Then operate 4 bytes at a time.
  for (; len >= sizeof(uint32_t); len -= sizeof(uint32_t), buf += sizeof(uint32_t)) {
    crc = _mm_crc32_u32(crc, *(uint32_t *) (buf));
  }
  // Then process at most 3 more bytes.
  for (; len >= 1; len -= sizeof(uint8_t), buf++) {
    crc = _mm_crc32_u8(crc, *(uint32_t *) (buf));
  }
  return ~crc;
}

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


  ItRange() : s_(), e_() {
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
      return false;
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

};

template <typename U, size_t count>
ItRange<U*> RangeFromLitStr(U (&str)[count]) {
  return ItRange<U*>(str, str + count - 1);
}

template <typename U, size_t count>
ItRange<U*> RangeFromArray(U (&str)[count]) {
  return ItRange<U*>(str, str + count);
}

ItRange<uint8_t*> RangeFromBytes(void* start, size_t count) {
  auto s = reinterpret_cast<uint8_t*>(start);
  return ItRange<uint8_t*>(s, s + count);
}

ItRange<const uint8_t*> RangeFromBytes(const void* start, size_t count) {
  auto s = reinterpret_cast<const uint8_t*>(start);
  return ItRange<const uint8_t*>(s, s + count);
}

template <typename U>
std::string StringFromRange(const ItRange<U>& r) {
  return std::string(r.start(), r.end());
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
// HexASCII (converts a byte into a two-char readable representation.
//
static const char HexASCIITable[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

char* HexASCII(uint8_t byte, char* out) {
  *out++ = HexASCIITable[(byte >> 4) & 0x0F];
  *out++ = HexASCIITable[byte & 0x0F];
  return out;
}

///////////////////////////////////////////////////////////////////////////////
std::string HexASCIIStr(const plx::Range<const uint8_t>& r, char separator) {
  if (r.empty())
    return std::string();

  std::string str((r.size() * 3) - 1, separator);
  char* data = &str[0];
  for (size_t ix = 0; ix != r.size(); ++ix) {
    data = plx::HexASCII(r[ix], data);
    ++data;
  }
  return str;
}

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
  friend class FileView;

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

  size_t size_in_bytes() const {
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
// plx::OverflowKind
//
enum class OverflowKind {
  None,
  Positive,
  Negative,
};

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

short NextInt(char value) {
  return short(value);
}

int NextInt(short value) {
  return int(value);
}

long long NextInt(int value) {
  return long long(value);
}

long long NextInt(long value) {
  return long long(value);
}

long long NextInt(long long value) {
  return value;
}

short NextInt(unsigned char value) {
  return short(value);
}

int NextInt(unsigned short value) {
  return int(value);
}

long long NextInt(unsigned int value) {
  return long long(value);
}

long long NextInt(unsigned long value) {
  return long long(value);
}

long long NextInt(unsigned long long value) {
  if (static_cast<long long>(value) < 0LL)
    throw plx::OverflowException(__LINE__, plx::OverflowKind::Positive);
  return long long(value);
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
bool PlatformCheck() {
  __if_exists(plex_sse42_support) {
    if (!plx::CpuId().sse42())
      return false;
  }
  return true;
};

///////////////////////////////////////////////////////////////////////////////
// plx::DecodeString (decodes a json-style encoded string)
//
std::string DecodeString(plx::Range<const char>& range) {
  if (range.empty())
    return std::string();
  if (range[0] != '\"') {
    auto r = plx::RangeFromBytes(range.start(), 1);
    throw plx::CodecException(__LINE__, &r);
  }

  std::string s;
  for (;;) {
    auto text_start = range.start();
    while (range.advance(1) > 0) {
      auto c = range.front();
      if (c < 32) {
        throw plx::CodecException(__LINE__, nullptr);
      } else {
        switch (c) {
          case '\"' : break;
          case '\\' : goto escape;
          default: continue;
        }
      }
      s.append(++text_start, range.start());
      range.advance(1);
      return s;
    }
    // Reached the end of range before a (").
    throw plx::CodecException(__LINE__, nullptr);

  escape:
    s.append(++text_start, range.start());
    if (range.advance(1) <= 0)
      throw plx::CodecException(__LINE__, nullptr);

    switch (range.front()) {
      case '\"':  s.push_back('\"'); break;
      case '\\':  s.push_back('\\'); break;
      case '/':   s.push_back('/');  break;
      case 'b':   s.push_back('\b'); break;
      case 'f':   s.push_back('\f'); break;
      case 'n':   s.push_back('\n'); break;
      case 'r':   s.push_back('\r'); break;
      case 't':   s.push_back('\t'); break;   //$$ missing \u (unicode).
      default: {
        auto r = plx::RangeFromBytes(range.start() - 1, 2);
        throw plx::CodecException(__LINE__, &r);
      }
    }
  }
}

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

char32_t DecodeUTF8(plx::Range<const unsigned char>& ir) {
  if (!ir.valid() || (ir.size() == 0))
    throw plx::CodecException(__LINE__, nullptr);

  unsigned char fst = ir[0];
  if (!(fst & 0x80)) {
    // One byte sequence, so we are done.
    ir.advance(1);
    return fst;
  }

  if ((fst & 0xC0) != 0xC0)
    throw plx::CodecException(__LINE__, &ir);

  uint32_t d = fst;
  fst <<= 1;

  for (unsigned int i = 1; (i != 3) && (ir.size() > i); ++i) {
    unsigned char tmp = ir[i];

    if ((tmp & 0xC0) != 0x80)
      throw plx::CodecException(__LINE__, &ir);

    d = (d << 6) | (tmp & 0x3F);
    fst <<= 1;

    if (!(fst & 0x80)) {
      d &= Utf8BitMask[i];

      // overlong check.
      if ((d & ~Utf8BitMask[i - 1]) == 0)
        throw plx::CodecException(__LINE__, &ir);

      // surrogate check.
      if (i == 2) {
        if ((d >= 0xD800 && d <= 0xDFFF) || d > 0x10FFFF)
          throw plx::CodecException(__LINE__, &ir);
      }

      ir.advance(i + 1);
      return d;
    }

  }
  throw plx::CodecException(__LINE__, &ir);
}

///////////////////////////////////////////////////////////////////////////////
// plx::ParseJsonValue (converts a JSON string into a JsonValue)
//
plx::JsonValue ParseJsonValue(plx::Range<const char>& range);

namespace JsonImp {
template <typename StrT>
bool Consume(plx::Range<const char>& r, StrT&& str) {
  auto c = r.starts_with(plx::RangeFromLitStr(str));
  if (c) {
    r.advance(c);
    return true;
  }
  else {
    return (c != 0);
  }
}

bool IsNumber(plx::Range<const char>&r) {
  if ((r.front() >= '0') && (r.front() <= '9'))
    return true;
  if ((r.front() == '-') || (r.front() == '+'))
    return true;
  if (r.front() == '.')
    return true;
  return false;
}

plx::JsonValue ParseArray(plx::Range<const char>& range) {
  if (range.empty())
    throw plx::CodecException(__LINE__, NULL);
  if (range.front() != '[')
    throw plx::CodecException(__LINE__, NULL);

  JsonValue value(plx::JsonType::ARRAY);
  range.advance(1);

  for (;!range.empty();) {
    range = plx::SkipWhitespace(range);

    if (range.front() == ',') {
      if (range.advance(1) <= 0)
        break;
      range = plx::SkipWhitespace(range);
    }

    if (range.front() == ']') {
      range.advance(1);
      return value;
    }

    value.push_back(ParseJsonValue(range));
  }

  auto r = plx::RangeFromBytes(range.start(), range.size());
  throw plx::CodecException(__LINE__, &r);
}

plx::JsonValue ParseNumber(plx::Range<const char>& range) {
  size_t pos = 0;
  auto num = plx::StringFromRange(range);

  auto iv = std::stoll(num, &pos);
  if ((range[pos] != 'e') && (range[pos] != 'E') && (range[pos] != '.')) {
    range.advance(pos);
    return iv;
  }

  auto dv = std::stod(num, &pos);
  range.advance(pos);
  return dv;
}

plx::JsonValue ParseObject(plx::Range<const char>& range) {
  if (range.empty())
    throw plx::CodecException(__LINE__, NULL);
  if (range.front() != '{')
    throw plx::CodecException(__LINE__, NULL);

  JsonValue obj(plx::JsonType::OBJECT);
  range.advance(1);

  for (;!range.empty();) {
    if (range.front() == '}') {
      range.advance(1);
      return obj;
    }

    range = plx::SkipWhitespace(range);
    auto key = plx::DecodeString(range);

    range = plx::SkipWhitespace(range);
    if (range.front() != ':')
      throw plx::CodecException(__LINE__, nullptr);
    if (range.advance(1) <= 0)
      throw plx::CodecException(__LINE__, nullptr);

    range = plx::SkipWhitespace(range);
    obj[key] = ParseJsonValue(range);

    range = plx::SkipWhitespace(range);
    if (range.front() == ',') {
      if (range.advance(1) <= 0)
        break;
      range = plx::SkipWhitespace(range);
    }
  }
  throw plx::CodecException(__LINE__, nullptr);
}

} // namespace JsonImp

plx::JsonValue ParseJsonValue(plx::Range<const char>& range) {
  range = plx::SkipWhitespace(range);
  if (range.empty())
    throw plx::CodecException(__LINE__, NULL);

  if (range.front() == '{')
    return JsonImp::ParseObject(range);
  if (range.front() == '\"')
    return plx::DecodeString(range);
  if (range.front() == '[')
    return JsonImp::ParseArray(range);
  if (JsonImp::Consume(range, "true"))
    return true;
  if (JsonImp::Consume(range, "false"))
    return false;
  if (JsonImp::Consume(range, "null"))
    return nullptr;
  if (JsonImp::IsNumber(range))
    return JsonImp::ParseNumber(range);

  auto r = plx::RangeFromBytes(range.start(), range.size());
  throw plx::CodecException(__LINE__, &r);
}

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
}

void Test_PlatformCheck::Exec() {
  CheckEQ(plx::PlatformCheck(), true);
}

void Test_Range::Exec() {
  // Empty iterator is empty.
  plx::Range<const char> range1;
  CheckEQ(range1.empty(), true);
  CheckEQ(range1.valid(), true);
  CheckEQ(range1.size(), size_t(0));

  // The range so defined includes the null character.
  const char txt1[] = "abcdefghijklmnopqrstuv";
  const plx::Range<const char> range2(txt1, sizeof(txt1));
  CheckEQ(range2.empty(), false);
  CheckEQ(range2.valid(), true);
  CheckEQ(range2.size(), sizeof(txt1));
  CheckEQ(range2.front(), 'a');
  CheckEQ(range2.back(), '\0');
  CheckEQ(range2[2], 'c');

  char txt2[5];
  auto copied = range2.CopyToArray(txt2);
  CheckEQ(memcmp(txt1, txt2, 5) == 0, true);
  CheckEQ(copied, 5UL);

  std::array<char, 3> txt3;
  copied = range2.CopyToArray(txt3);
  CheckEQ(copied, 3UL);

  plx::Range<const char> range3(range2);
  CheckEQ(range3.advance(3) > 0, true);
  CheckEQ(range3.size(), range2.size() - 3);
  CheckEQ(range3.front(), 'd');

  CheckEQ(range3.advance(50) < 0, true);
  CheckEQ(range3.front(), 'd');

  auto range4 = plx::RangeFromLitStr("12345678");
  CheckEQ(range4.size(), 8);
  CheckEQ(range4[0], '1');
  CheckEQ(range4[7], '8');

  unsigned char data[] = {0x11, 0x22, 0x33};
  auto range5 = plx::RangeFromArray(data);
  CheckEQ(range5.size(), 3);
  CheckEQ(range5[0], 0x11);
  CheckEQ(range5[2], 0x33);

  auto range6 = range4;
  CheckEQ(range4.equals(range6), true);
  CheckEQ(range6.equals(range4), true);
  range6.advance(1);
  CheckEQ(range6.equals(range4), false);
  CheckEQ(range4.equals(range6), false);

  auto range7 = plx::RangeFromLitStr("12");
  CheckEQ(range4.starts_with(range7), 2);
  CheckEQ(range7.starts_with(range4), 0);

  CheckEQ(range7.advance(2), 0);
  try {
    auto c = range7.front();
    __debugbreak();
  } catch (plx::RangeException&) {
  }
}

void Test_CpuId::Exec() {
  plx::CpuId cpu_id;

  auto s = cpu_id.stepping();
  auto m = cpu_id.model();
  auto f = cpu_id.family();

  CheckGT(s, int(1));
  CheckLT(s, int(16));

  CheckGT(m, int(2));
  CheckLT(m, int(16));

  CheckGT(f, int(3));
  CheckLT(f, int(16));

  CheckEQ(cpu_id.sse(), true);
  CheckEQ(cpu_id.mmx(), true);
  CheckEQ(cpu_id.sse2(), true);
  CheckEQ(cpu_id.sse3(), true);
}

template <typename T>
std::pair<T, T> MinMaxForType() {
  return std::make_pair(
      std::numeric_limits<T>::min(),
      std::numeric_limits<T>::max());
}

template <typename T, typename V>
bool EqNum(T t, V v) {
  return std::to_string(t) == std::to_string(v);
}
template <typename T>
bool EqNum(T t, T v) {
  return (t == v);
}

void Test_To_Integer::Exec() {
  {
    auto mm = MinMaxForType<char>();
    CheckEQ(EqNum(plx::To<short>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<short>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<int>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<int>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<long>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<long>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<long long>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<long long>(mm.second), mm.second), true);

    CheckEQ(EqNum(plx::To<unsigned char>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<unsigned short>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<unsigned int>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<unsigned long>(mm.second), mm.second), true);
  }

  {
    auto mm = MinMaxForType<short>();
    CheckEQ(EqNum(plx::To<int>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<int>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<long>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<long>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<long long>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<long long>(mm.second), mm.second), true);

    CheckEQ(EqNum(plx::To<unsigned short>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<unsigned int>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<unsigned long>(mm.second), mm.second), true);
  }

  {
    auto mm = MinMaxForType<int>();
    CheckEQ(EqNum(plx::To<long>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<long>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<long long>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<long long>(mm.second), mm.second), true);

    CheckEQ(EqNum(plx::To<unsigned int>(mm.second), mm.second), true);
    CheckEQ(EqNum(plx::To<unsigned long>(mm.second), mm.second), true);
  }

  {
    auto mm = MinMaxForType<long>();
    CheckEQ(EqNum(plx::To<long long>(mm.first), mm.first), true);
    CheckEQ(EqNum(plx::To<long long>(mm.second), mm.second), true);

    CheckEQ(EqNum(plx::To<unsigned long>(mm.second), mm.second), true);
  }

  {
    auto mm = MinMaxForType<unsigned char>();
    CheckEQ(EqNum(plx::To<char>(mm.first), 0), true);

    try {
      auto x = plx::To<char>(mm.second);  __debugbreak();
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Positive);
    }
  }

  {
    auto mm = MinMaxForType<unsigned short>();
    CheckEQ(EqNum(plx::To<char>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<short>(mm.first), 0), true);

    int c = 0;
  loop1:
    try {
      switch (c) {
      case 0: { auto a = plx::To<char>(mm.second);  __debugbreak(); }
      case 1: { auto b = plx::To<short>(mm.second); __debugbreak(); }
        default: ;
      }
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Positive);
      ++c; goto loop1;
    }
    CheckEQ(c, 2);
  }

  {
    auto mm = MinMaxForType<unsigned int>();
    CheckEQ(EqNum(plx::To<char>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<short>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<int>(mm.first), 0), true);

    int c = 0;
  loop2:
    try {
      switch (c) {
      case 0: { auto a = plx::To<char>(mm.second);  __debugbreak(); }
      case 1: { auto b = plx::To<short>(mm.second); __debugbreak(); }
      case 2: { auto b = plx::To<int>(mm.second); __debugbreak(); }
        default: ;
      }
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Positive);
      ++c; goto loop2;
    }
    CheckEQ(c,3);
  }

  {
    auto mm = MinMaxForType<unsigned long>();
    CheckEQ(EqNum(plx::To<char>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<short>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<int>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<long>(mm.first), 0), true);

    int c = 0;
  loop3:
    try {
      switch (c) {
      case 0: { auto a = plx::To<char>(mm.second);  __debugbreak(); }
      case 1: { auto b = plx::To<short>(mm.second); __debugbreak(); }
      case 2: { auto b = plx::To<int>(mm.second); __debugbreak(); }
      case 3: { auto b = plx::To<long>(mm.second); __debugbreak(); }
        default: ;
      }
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Positive);
      ++c; goto loop3;
    }
    CheckEQ(c,4);
  }

  {
    auto mm = MinMaxForType<unsigned long long>();
    CheckEQ(EqNum(plx::To<char>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<short>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<int>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<long>(mm.first), 0), true);
    CheckEQ(EqNum(plx::To<long long>(mm.first), 0), true);

    int c = 0;
  loop4:
    try {
      switch (c) {
      case 0: { auto a = plx::To<char>(mm.second);  __debugbreak(); }
      case 1: { auto b = plx::To<short>(mm.second); __debugbreak(); }
      case 2: { auto b = plx::To<int>(mm.second); __debugbreak(); }
      case 3: { auto b = plx::To<long>(mm.second); __debugbreak(); }
      case 4: { auto b = plx::To<long long>(mm.second); __debugbreak(); }
        default: ;
      }
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Positive);
      ++c; goto loop4;
    }
    CheckEQ(c,5);
  }

  {
    auto mm = MinMaxForType<short>();
    int c = 0;
  loop5:
    try {
      switch (c) {
      case 0: { auto a = plx::To<unsigned char>(mm.first);  __debugbreak(); }
      case 1: { auto b = plx::To<unsigned short>(mm.first); __debugbreak(); }
        default: ;
      }
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Negative);
      ++c; goto loop5;
    }
    CheckEQ(c, 2);
  }

  {
    auto mm = MinMaxForType<int>();
    int c = 0;
  loop6:
    try {
      switch (c) {
      case 0: { auto a = plx::To<unsigned char>(mm.first);  __debugbreak(); }
      case 1: { auto b = plx::To<unsigned short>(mm.first); __debugbreak(); }
      case 2: { auto b = plx::To<unsigned int>(mm.first); __debugbreak(); }
        default: ;
      }
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Negative);
      ++c; goto loop6;
    }
    CheckEQ(c,3);
  }

  {
    auto mm = MinMaxForType<long>();
    int c = 0;
  loop7:
    try {
      switch (c) {
      case 0: { auto a = plx::To<unsigned char>(mm.first);  __debugbreak(); }
      case 1: { auto b = plx::To<unsigned short>(mm.first); __debugbreak(); }
      case 2: { auto b = plx::To<unsigned int>(mm.first); __debugbreak(); }
      case 3: { auto b = plx::To<unsigned long>(mm.first); __debugbreak(); }
        default: ;
      }
    } catch (plx::OverflowException& e) {
      CheckEQ(e.kind(), plx::OverflowKind::Negative);
      ++c; goto loop7;
    }
    CheckEQ(c,4);
  }
}

double TestFnReturnDouble1() {
  return 1.0;
}

void Test_ScopeGuard::Exec() {
  {
    auto sc1 = plx::MakeGuard(TestFnReturnDouble1);
  }

  std::vector<int> v1 = {1, 2};
  {
    auto g = plx::MakeGuard(std::bind(&std::vector<int>::pop_back, &v1));
  }
  CheckEQ(v1.size(), 1);
  {
    auto g = plx::MakeGuard([&] { v1.push_back(5); });
  }
  CheckEQ(v1.size(), 2);
  {
    auto g = plx::MakeGuard([&] { v1.push_back(4); });
    v1.pop_back();
    g.dismiss();
  }
  CheckEQ(v1.size(), 1);
}

void Test_Utf8decode::Exec() {
  {
    const unsigned char t[] = "\0";
    plx::Range<const unsigned char> r(t, 1);
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0);
  }

  {
    const unsigned char t[] = "the lazy.";
    plx::Range<const unsigned char> r(t, sizeof(t) - 1);
    std::u32string result;
    while (r.size()) {
      auto c = plx::DecodeUTF8(r);
      result.push_back(c);
    }
    std::u32string expected = { 't','h','e',' ','l','a','z','y','.'};
    CheckEQ(result == expected, true);
  }

  {
    // first two byte code U+0080 is 0xC2 0x80
    const unsigned char t[] = {0xC2, 0x80};
    plx::Range<const unsigned char> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0x80);
  }

  {
    // unicode U+00A9 (copyright sign) is 0xC2 0xA9
    const unsigned char t[] = {0xC2, 0xA9};
    plx::Range<const unsigned char> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0xA9);
  }

  {
    // first tree byte code U+0800 (samaritan alaf) is
    const unsigned char t[] = {0xE0, 0xA0, 0x80};
    plx::Range<const unsigned char> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0x800);
  }

  {
    // last 3 byte code U+FFFF (non character) is 0xE2 0x89 0xA0
    const unsigned char t[] = {0xEF, 0xBF, 0xBF};
    plx::Range<const unsigned char> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0xFFFF);
  }

  {
    // unicode U+2260 (not equal to) 0xE2 0x89 0xA0
    const unsigned char t[] = {0xE2, 0x89, 0xA0};
    plx::Range<const unsigned char> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0x2260);
  }

  {
    // greek word 'kosme'.
    const unsigned char t[] =
        {0xCE,0xBA,0xE1,0xBD,0xB9,0xCF,0x83,0xCE,0xBC,0xCE,0xB5};
    plx::Range<const unsigned char> r(t, sizeof(t));
    std::u32string result;
    while (r.size()) {
      auto c = plx::DecodeUTF8(r);
      result.push_back(c);
    }
    CheckEQ(result.size() == 5, true);
  }

   {
    // overlong form #1 for U+000A (line feed) 0xC0 0x8A
    const unsigned char t[] = {0xC0, 0x8A};
    plx::Range<const unsigned char> r(t, sizeof(t));
    try {
      auto c = plx::DecodeUTF8(r);
      __debugbreak();
    } catch (plx::CodecException& e) {
      CheckEQ(e.bytes(), "C0,8A");
    }
  }

  {
    // overlong form #2 for U+000A (line feed) 0xE0 0x80 0x8A
    const unsigned char t[] = {0xE0, 0x80, 0x8A};
    plx::Range<const unsigned char> r(t, sizeof(t));
    try {
      auto c = plx::DecodeUTF8(r);
      __debugbreak();
    } catch (plx::CodecException& e) {
      CheckEQ(e.bytes(), "E0,80,8A");
    }
  }

  {
    // overlong form #3 for U+000A (line feed) 0xF0 0x80 0x80 0x8A
    const unsigned char t[] = {0xF0, 0x80, 0x80, 0x8A};
    plx::Range<const unsigned char> r(t, sizeof(t));
    try {
      auto c = plx::DecodeUTF8(r);
      __debugbreak();
    } catch (plx::CodecException& e) {
      CheckEQ(e.bytes(), "F0,80,80,8A");
    }
  }

  {
    // overlong form #4 for U+000A (line feed) 0xF8 0x80 0x80 0x80 0x8A
    const unsigned char t[] = {0xF8, 0x80, 0x80, 0x80, 0x8A};
    plx::Range<const unsigned char> r(t, sizeof(t));
    try {
      auto c = plx::DecodeUTF8(r);
      __debugbreak();
    } catch (plx::CodecException& e) {
      CheckEQ(e.bytes(), "F8,80,80,80,8A");
    }
  }
}

void Test_JsonValue::Exec() {
  auto sj = sizeof(plx::JsonValue);
  plx::JsonValue value("most likely");
  plx::JsonValue obj1(plx::JsonType::OBJECT);
  obj1["foo"] = value;
  obj1["bar"] = plx::JsonValue("aruba");
  plx::JsonValue obj2(obj1);
  obj2["sun"] = "screen";
  obj2["lava"] = 333;
  plx::JsonValue obj3 = {22, 34, 77, 11, 55};
  plx::JsonValue obj4 = { "sun", 12, false, "rum", obj2 };
  plx::JsonValue obj5 = 12.0002;
  plx::JsonValue obj6 = nullptr;
}

void Test_Hex::Exec() {
  char out[256];
  char* pos = out;
  for (size_t ix = 0; ix != _countof(out) / 2; ix++) {
    plx::HexASCII(plx::To<uint8_t>(ix), pos);
    pos += 2;
  }
  char res[] = "000102030405060708090A0B0C0D0E0F";
  char* oo = out;
  int times = 8;
  do {
    CheckEQ(memcmp(res, oo, _countof(res) - 1), 0);
    int pos = 0;
    for (auto& x : res) {
      if (pos % 2 == 0)
        ++x;
      ++pos;
    }
    oo = oo + _countof(res) -1 ;
  } while (--times);

  const uint8_t data[] = {0xFF, 0xFE, 0xAA, 0xBB};
  CheckEQ(plx::HexASCIIStr(plx::RangeFromArray(data), '-'), "FF-FE-AA-BB");
}

void Test_Whitespace::Exec() {
  auto r = plx::RangeFromLitStr("  a\t cde f ");
  r = plx::SkipWhitespace(r);
  CheckEQ(r.front(), 'a');
  CheckEQ(r.size(), 9);
  r.advance(1);
  r = plx::SkipWhitespace(r);
  CheckEQ(r.front(), 'c');
  CheckEQ(r.size(), 6);
  r = plx::SkipWhitespace(r);
  CheckEQ(r.size(), 6);
  r.advance(3);
  r = plx::SkipWhitespace(r);
  CheckEQ(r.size(), 2);
  r.advance(1);
  r = plx::SkipWhitespace(r);
  CheckEQ(r.size(), 0);
}

void Test_DecodeString::Exec() {
  {
    auto r = plx::RangeFromLitStr(R"("jumps over the moon")");
    auto dec = plx::DecodeString(r);
    CheckEQ(dec, "jumps over the moon");
  }
  {
    auto r = plx::RangeFromLitStr(R"("ides\tof\tmarch")");
    auto dec = plx::DecodeString(r);
    CheckEQ(dec, "ides\tof\tmarch");
  }
  {
    auto r = plx::RangeFromLitStr(R"("\nreturn\n")");
    auto dec = plx::DecodeString(r);
    CheckEQ(dec, "\nreturn\n");
  }
  {
    auto r = plx::RangeFromLitStr(R"("\\\\\\\\")");
    auto dec = plx::DecodeString(r);
    CheckEQ(dec, "\\\\\\\\");
  }
  {
    auto r = plx::RangeFromLitStr(R"("some\rone"$1000)");
    auto dec = plx::DecodeString(r);
    CheckEQ(dec, "some\rone");
    CheckEQ(r[0], '$');
    CheckEQ(r.size(), 5);
  }
  {
    auto r = plx::RangeFromLitStr(R"("missing end)");
    try {
      auto dec = plx::DecodeString(r);
      __debugbreak();
    } catch (plx::CodecException& ) {
    }
  }
  {
    auto r = plx::RangeFromLitStr("\"a \0 b\"");
    try {
      auto dec = plx::DecodeString(r);
      __debugbreak();
    } catch (plx::CodecException& ) {
    }
  }
  {
    auto r = plx::RangeFromLitStr(R"("one \)");
    try {
      auto dec = plx::DecodeString(r);
      __debugbreak();
    } catch (plx::CodecException& ) {
    }
  }
}

void Test_Parse_JSON::Exec() {
  {
    auto json = plx::RangeFromLitStr(" true");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::BOOL);
    CheckEQ(value.get_bool(), true);
  }
  {
    auto json = plx::RangeFromLitStr("false ");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::BOOL);
    CheckEQ(value.get_bool(), false);
  }
  {
    auto json = plx::RangeFromLitStr("\"some string\"");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::STRING);
    CheckEQ(value.get_string(), "some string");
  }
  {
    auto json = plx::RangeFromLitStr("null");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::NULLT);
  }
  {
    auto json = plx::RangeFromLitStr("555555555555");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::INT64);
    CheckEQ(value.get_int64(), 555555555555LL);
  }
  {
    auto json = plx::RangeFromLitStr("-2");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::INT64);
    CheckEQ(value.get_int64(), -2LL);
  }
  {
    auto json = plx::RangeFromLitStr("-22.01");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::DOUBLE);
    CheckEQ(value.get_double(), -22.01);
  }
  {
    auto json = plx::RangeFromLitStr(R"([true])");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::ARRAY);
    CheckEQ(value.size(), 1);
    CheckEQ(value[0].type(), plx::JsonType::BOOL);
  }
  {
    auto json = plx::RangeFromLitStr(R"(["novus", "ordo", "seclorum"])");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::ARRAY);
    CheckEQ(value.size(), 3);
    for (size_t ix = 0; ix != 3; ++ix) {
      CheckEQ(value[ix].type(), plx::JsonType::STRING);
    }
  }
  {
    auto json = plx::RangeFromLitStr(R"([ 3.1415,0,2.999,30000000 ])");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::ARRAY);
    CheckEQ(value.size(), 4);
    CheckEQ(value[0].type(), plx::JsonType::DOUBLE);
    CheckEQ(value[1].type(), plx::JsonType::INT64);
    CheckEQ(value[2].type(), plx::JsonType::DOUBLE);
    CheckEQ(value[3].type(), plx::JsonType::INT64);
  }

  {
    auto json = plx::RangeFromLitStr(R"({})");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::OBJECT);
  }
  {
    auto json = plx::RangeFromLitStr(R"({"ip": "8.8.8.8"})");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::OBJECT);
    CheckEQ(value.size(), 1);
  }
  {
    auto json = plx::RangeFromLitStr(R"(
      {
         "kind": "object",
         "empty": false,
         "parse_time_ns": 19608,
         "validate": true,
         "size": 1
      }
    )");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::OBJECT);
    CheckEQ(value.size(), 5);
    CheckEQ(value.has_key("kind"), true);
    CheckEQ(value.has_key("color"), false);
    CheckEQ(value.has_key("size"), true);
  }
  {
    auto json = plx::RangeFromLitStr(R"([{"":[{}]},{"":[{}]}])");
    auto value = plx::ParseJsonValue(json);
    CheckEQ(value.type(), plx::JsonType::ARRAY);
    CheckEQ(value.size(), 2);
    auto& e0 = value[0];
    auto& e1 = value[1];
    CheckEQ(e0.type(), plx::JsonType::OBJECT);
    CheckEQ(e0.size(), 1);
    CheckEQ(e1.type(), plx::JsonType::OBJECT);
    CheckEQ(e1.size(), 1);
    auto& ne0 = e0[""];
    auto& ne1 = e1[""];
    CheckEQ(ne0.type(), plx::JsonType::ARRAY);
    CheckEQ(ne0.size(), 1);
    CheckEQ(ne1.type(), plx::JsonType::ARRAY);
    CheckEQ(ne1.size(), 1);
    auto& me0 = ne0[0];
    auto& me1 = ne1[0];
    CheckEQ(me0.type(), plx::JsonType::OBJECT);
    CheckEQ(me0.size(), 0);
    CheckEQ(me1.type(), plx::JsonType::OBJECT);
    CheckEQ(me1.size(), 0);
  }
}

void Test_CRC32C::Exec() {
  plx::CpuId cpu_id;
  CheckEQ(cpu_id.sse42(), true);

  {
    char ts[] = "123456789";
    CheckEQ(plx::CRC32C(0, ts, sizeof(ts) - 1), 0xE3069283);
  }
  {
    char ts[] = " the lazy fox jumps over";
    CheckEQ(plx::CRC32C(0, ts + 1, sizeof(ts) - 2), 0xEFB0976C);
  }
  {
    char ts[] = "0";
    CheckEQ(plx::CRC32C(0, ts, sizeof(ts) - 1), 0x629E1AE0);
  }
}

void Test_FilePath::Exec() {
  plx::FilePath fp1(L"C:\\1\\2\\file.ext");
  CheckEQ(std::wstring(L"C:\\1\\2\\file.ext"), fp1.raw());
  CheckEQ(std::wstring(L"file.ext"), fp1.leaf());
  CheckEQ(fp1.has_drive(), true);
  CheckEQ(fp1.is_drive(), false);

  auto fp2 = fp1.parent();
  CheckEQ(std::wstring(L"C:\\1\\2"), fp2.raw());
  CheckEQ(std::wstring(L"2"), fp2.leaf());

  auto fp3 = fp2.parent();
  CheckEQ(std::wstring(L"C:\\1"), fp3.raw());
  CheckEQ(std::wstring(L"1"), fp3.leaf());

  auto fp4 = fp3.parent();
  CheckEQ(std::wstring(L"C:"), fp4.raw());
  CheckEQ(fp4.is_drive(), true);
  CheckEQ(std::wstring(L""), fp4.leaf());

  auto fp5 = fp4.parent();
  CheckEQ(std::wstring(L""), fp5.raw());

  auto fp6 = fp4.append(L"one").append(L"two");
  CheckEQ(std::wstring(L"C:\\one\\two"), fp6.raw());

  auto fp7 = fp5.append(L"4\\5");
  CheckEQ(std::wstring(L"4\\5"), fp7.raw());
}

void Test_File::Exec() {
  plx::FileParams par1;
  CheckEQ(par1.can_modify(), false);
  CheckEQ(par1.exclusive(), false);

  auto par2 = plx::FileParams::Append_SharedRead();
  CheckEQ(par2.can_modify(), true);
  CheckEQ(par2.exclusive(), false);

  auto par3 = plx::FileParams::Read_SharedRead();
  CheckEQ(par3.can_modify(), false);
  CheckEQ(par3.exclusive(), false);

  // Current directory should be $(ProjectDir) which is src\utests.
  plx::FilePath fp1(L"data\\file_io\\data_001.txt");
  {
    plx::File f = plx::File::Create(fp1, par3, plx::FileSecurity());
    CheckEQ(f.is_valid(), true);
    CheckEQ(f.status(), plx::File::existing);
    CheckEQ(f.size_in_bytes(), 2048UL);
  }
  {
    auto par = plx::FileParams::Directory_ShareAll();
    plx::File f = plx::File::Create(fp1.parent(), par, plx::FileSecurity());
    CheckEQ(f.is_valid(), true);
    CheckEQ(f.status(), plx::File::directory | plx::File::existing);
  }

}
