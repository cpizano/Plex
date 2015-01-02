// This is the plex precompiled header, not the same as the VC precompiled header.

#pragma once
#define NOMINMAX

#include <SDKDDKVer.h>





#include <Shlobj.h>
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
#include <thread>
#include <memory>
#include <unordered_map>
#include <vector>
#include <stdint.h>
#include <stdarg.h>

const int plex_sse42_support = 1;

const int plex_cpuid_support = 1;

const int plex_vista_support = 1;
#include <windows.h>









///////////////////////////////////////////////////////////////////////////////
// plx::ArgInfo, ArgType.
//
namespace plx {

enum class ArgType : int {
  boolean,
  signed_int,
  unsigned_int,
  string_zt,
  pointer
};

struct ArgInfo {
  union {
    // An integer-like value.
    uint64_t integer;
    // A C-style text string.
    const char* str;
    // A pointer to an arbitrary object.
    const void* ptr;
  };

  const unsigned char width;
  const ArgType type;
  ArgInfo(bool v) : integer(v), width(0), type(ArgType::boolean) {}

  ArgInfo(signed char v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed short v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed int v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed long v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed long long v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}

  ArgInfo(unsigned char v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned short v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned int v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned long v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned long long v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}

  // A C-style text string.
  ArgInfo(const char* s) : str(s), width(sizeof(*s)), type(ArgType::string_zt) { }
  ArgInfo(char* s)       : str(s), width(sizeof(*s)), type(ArgType::string_zt) { }

  // Any pointer value that can be cast to a "void*".
  template<class T>
  ArgInfo(T* p) : ptr((void*)p), type(ArgType::pointer) { }

};


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
// plx::CRC32C (computes CRC-32 checksum, SSE4 accelerated)
// Polinomal 0x1EDC6F41 aka iSCSI CRC. This gives a 10^-41 probabilty of not
// detecting a 3-bit burst error and 10^-40 for sporadic one bit errors.
//
#pragma comment(user, "plex.define=plex_sse42_support")

uint32_t CRC32C(uint32_t crc, const uint8_t *buf, size_t len) ;


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

  It begin() const {
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

  bool contains(const uint8_t* ptr) const {
    return ((ptr >= reinterpret_cast<uint8_t*>(s_)) &&
            (ptr < reinterpret_cast<uint8_t*>(e_)));
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

  void extend(size_t count) {
    e_ += count;
  }

  ItRange<const uint8_t*> const_bytes() const {
    auto s = reinterpret_cast<const uint8_t*>(s_);
    auto e = reinterpret_cast<const uint8_t*>(e_);
    return ItRange<const uint8_t*>(s, e);
  }

  ItRange<uint8_t*> bytes() const {
    auto s = reinterpret_cast<uint8_t*>(s_);
    auto e = reinterpret_cast<uint8_t*>(e_);
    return ItRange<uint8_t*>(s, e);
  }

  ItRange<It> slice(size_t start, size_t count = 0) const {
    return ItRange<It>(s_ + start,
                       count ? (s_ + start + count) : e_ );
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

template <typename U>
ItRange<U*> RangeFromVector(std::vector<U>& vec, size_t len = 0) {
  auto s = &vec[0];
  return ItRange<U*>(s, len ? s + len : s + vec.size());
}

template <typename U>
ItRange<const U*> RangeFromVector(const std::vector<U>& vec, size_t len = 0) {
  auto s = &vec[0];
  return ItRange<const U*>(s, len ? s + len : s + vec.size());
}

ItRange<uint8_t*> RangeFromBytes(void* start, size_t count) ;

ItRange<const uint8_t*> RangeFromBytes(const void* start, size_t count) ;

ItRange<const uint8_t*> RangeFromString(const std::string& str) ;

ItRange<uint8_t*> RangeFromString(std::string& str) ;

ItRange<const uint16_t*> RangeFromString(const std::wstring& str) ;

ItRange<uint16_t*> RangeFromString(std::wstring& str) ;

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
// plx::Range  (alias for ItRange<T*>)
//
template <typename T>
using Range = plx::ItRange<T*>;



///////////////////////////////////////////////////////////////////////////////
// HexASCII (converts a byte into a two-char readable representation.
//
static const char HexASCIITable[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

char* HexASCII(uint8_t byte, char* out) ;


///////////////////////////////////////////////////////////////////////////////
// plx::MemoryException
//
class MemoryException : public plx::Exception {
  void* address_;

public:
  MemoryException(int line, void* address)
      : Exception(line, "Memory"), address_(address) {
    PostCtor();
  }
  void* address() const { return address_; }
};


///////////////////////////////////////////////////////////////////////////////
std::string HexASCIIStr(const plx::Range<const uint8_t>& r, char separator) ;


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
    std::unique_ptr<uint8_t[]> data;
    Item(size_t size)
        : size(size), data(new uint8_t[size]) {
    }

    Item(const Item& other)
        : size(other.size), data(new uint8_t[size]) {
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

  plx::Range<uint8_t> new_buffer(size_t size_bytes) {
    buffers_.emplace_back(size_bytes);
    auto start = &(buffers_.back().data)[0];
    return plx::Range<uint8_t>(start, start + size_bytes);
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

  plx::Range<uint8_t> get() {
    auto start = &(loop_it_->data)[0];
    return plx::Range<uint8_t>(start, start + loop_it_->size);
  }
};




///////////////////////////////////////////////////////////////////////////////
// ReaderWriterLock and its scoped unlockers.
//
class ScopedWriteLock {
  SRWLOCK* lock_;
  explicit ScopedWriteLock(SRWLOCK* lock) : lock_(lock) {}
  friend class ReaderWriterLock;

public:
  ScopedWriteLock() = delete;
  ScopedWriteLock(const ScopedWriteLock&) = delete;
  // void* operator new(std::size_t) =delete

  ~ScopedWriteLock() {
    ::ReleaseSRWLockExclusive(lock_);
  }
};

class ScopedReadLock {
  SRWLOCK* lock_;
  explicit ScopedReadLock(SRWLOCK* lock) : lock_(lock) {}
  friend class ReaderWriterLock;

public:
  ScopedReadLock() = delete;
  ScopedReadLock(const ScopedReadLock&) = delete;
  // void* operator new(std::size_t) =delete

  ~ScopedReadLock() {
    ::ReleaseSRWLockShared(lock_);
  }
};

// The SRW lock is implemented using a single pointer-sized atomic variable
// which can take on a number of different states, depending on the values
// of the low bits. The number of state transitions is pretty high, here are
// some common ones:
// - initial lock state: (0, ControlBits:0) -- An SRW lock starts with all
//   bits set to 0.
// - shared state: (ShareCount: n, ControlBits: 1) -- When there is no conflicting
//   exclusive acquire and the lock is held shared, the share count is stored
//   directly in the lock variable.
// - exclusive state: (ShareCount: 0, ControlBits: 1) -- When there is no
//   conflicting shared acquire or exclusive acquire, the lock has a low bit set
//   and nothing else.
// - contended case 1 : (WaitPtr:ptr, ControlBits: 3) -- When there is a conflict,
//   the threads that are waiting for the lock form a queue using data allocated
//   on the waiting threads' stacks. The lock variable stores a pointer to the
//   tail of the queue instead of a share count.

class ReaderWriterLock {
  char cache_line_pad_[64 - sizeof(SRWLOCK)];
  SRWLOCK lock_;

public:
  ReaderWriterLock() {
    ::InitializeSRWLock(&lock_);
  }

  ReaderWriterLock(ReaderWriterLock&) = delete;
  ReaderWriterLock& operator=(const ReaderWriterLock&) = delete;

  // Acquiring an exclusive lock when you don't know the initial state is a
  // single write to the lock word, to set the low bit (LOCK BTS instruction)
  // If you succeeded you can proceed into the locked region with no further
  // operations.

  ScopedWriteLock write_lock() {
    ::AcquireSRWLockExclusive(&lock_);
    return ScopedWriteLock(&lock_);
  }

  // Trying to acquire a shared lock is a more involved operation: You need to
  // first read the initial value of the lock variable to determine the old
  // share count, increment the share count you read, and then write the updated
  // value back conditionally with the LOCK CMPXCHG instruction.

  ScopedReadLock read_lock() {
    ::AcquireSRWLockShared(&lock_);
    return ScopedReadLock(&lock_);
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::GetAppDataPath
//
plx::FilePath GetAppDataPath(bool roaming) ;


///////////////////////////////////////////////////////////////////////////////
// plx::GetExePath
//
plx::FilePath GetExePath() ;


///////////////////////////////////////////////////////////////////////////////
// plx::Globals
//
template <class T> struct ServiceNumber;
class TestService; template <> struct ServiceNumber<TestService> { enum { id = 0 }; };
class VEHManager;  template <> struct ServiceNumber<VEHManager>  { enum { id = 1 }; };

class Globals {
  void* services_[2];

  static void* raw_get(int id, void** svcs = nullptr) {
    static void** services = svcs;
    return services[id];
  }

public:
  Globals() {
    memset(services_, 0, sizeof(services_));
    raw_get(0, services_);
  }

  template <typename Svc>
  void add_service(Svc* svc) {
    const int id = ServiceNumber<Svc>::id;
    services_[id] = svc;
  }

  template <typename Svc>
  static Svc* get() {
    const int id = ServiceNumber<Svc>::id;
    return reinterpret_cast<Svc*>(raw_get(id));
  }

};


///////////////////////////////////////////////////////////////////////////////
// plx::Hash_FNV1a_32  (nice hash function for strings used by c++ std)
// for short inputs is about 100 times faster than SHA1 and about 20 times
// faster for long inputs.
uint32_t Hash_FNV1a_32(const plx::Range<const uint8_t>& r) ;


///////////////////////////////////////////////////////////////////////////////
// plx::Hash_FNV1a_64  (nice hash function for strings used by c++ std)
// for short inputs is about 100 times faster than SHA1 and about 20 times
// faster for long inputs.
uint64_t Hash_FNV1a_64(const plx::Range<const uint8_t>& r) ;


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
// plx::DecodeString (decodes a json-style encoded string)
//
std::string DecodeString(plx::Range<const char>& range) ;


///////////////////////////////////////////////////////////////////////////////
// plx::BitSlicer allows you to slice sequential bits of a byte range.
// bits_ : the last extracted bits not given back to caller
// bit_count_ : how many valid bits in |bits_|.
// pos_ : the current byte position in |r_|.
//

class BitSlicer {
  const plx::Range<const unsigned char>& r_;
  unsigned long bits_;
  size_t bit_count_;
  size_t pos_;

  unsigned long range_check(bool* eor) {
    if (!eor)
      throw plx::RangeException(__LINE__, 0);
    *eor = true;
    return 0;
  }

public:
  BitSlicer(const plx::Range<const unsigned char>& r)
      : r_(r), bits_(0), bit_count_(0), pos_(0) {
  }

  unsigned long slice(int needed, bool* eor = nullptr) {
    if (needed > 23) {
      throw plx::RangeException(__LINE__, 0);
    }

    unsigned long value = bits_;

    while (bit_count_ < needed) {
      if (past_end())
        return range_check(eor);
      // accumulate 8 bits at a time.
      value |= static_cast<unsigned long>(r_[pos_++]) << bit_count_;
      bit_count_ += 8;
    }

    // store the uneeded bits from above. We will use them
    // for the next request.
    bits_ = value >> needed;
    bit_count_ -= needed;

    // mask out the bits above |needed|.
    return value & ((1L << needed) - 1);
  }

  bool past_end() const {
    return (pos_ == r_.size());
  }

  void discard_bits() {
    bit_count_ = 0;
    bits_ = 0;
  }

  void skip(size_t n) {
    discard_bits();
    pos_ += n;
  }

  uint32_t next_uint32() {
    auto rv = *reinterpret_cast<const uint32_t*>(&r_[pos_]);
    skip(sizeof(uint32_t));
    return rv;
  }

  plx::Range<const unsigned char> remaining_range() const {
    return plx::Range<const unsigned char>(r_.start() + pos_, r_.end());
  }

  size_t pos() const {
    return pos_;
  }

};



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
// plx::VEHManager
//
class VEHManager {

  struct Node {
    DWORD code;
    plx::Range<uint8_t> address;
    std::function<bool(EXCEPTION_RECORD*)> handler;
    Node(DWORD c, const plx::Range<uint8_t>& r, std::function<bool(EXCEPTION_RECORD*)>& f)
      : code(c), address(r), handler(f) { }
  };

  bool RunHandler(EXCEPTION_RECORD* er) {
    const uint8_t* address = nullptr;
    const DWORD code = er->ExceptionCode;

    if (code == EXCEPTION_ACCESS_VIOLATION) {
      address = reinterpret_cast<uint8_t*>(
          er->ExceptionInformation[1]);
    } else {
      address = reinterpret_cast<uint8_t*>(
          er->ExceptionAddress);
    }

    auto rl = lock_.read_lock();
    for (auto& node : handlers_) {
      if (!node.address.contains(address))
        continue;

      if (node.code == code) {
        if (node.handler(er))
          return true;
      }
    }

    return false;
  }

  static LONG __stdcall VecHandler(EXCEPTION_POINTERS* ep) {
    if (ep == nullptr || ep->ExceptionRecord == nullptr)
      return EXCEPTION_CONTINUE_SEARCH;
    if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
      return EXCEPTION_CONTINUE_SEARCH;
    bool handled = plx::Globals::get<VEHManager>()->RunHandler(ep->ExceptionRecord);
    return handled ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
  }

  plx::ReaderWriterLock lock_;
  std::vector<Node> handlers_;
  void* veh_handler_;

public:
  typedef std::function<bool(EXCEPTION_RECORD*)> HandlerFn;

  VEHManager() = default;
  VEHManager(const VEHManager&) = delete;
  VEHManager& operator=(const VEHManager&) = delete;

  void add_av_handler(const plx::Range<uint8_t>& address, HandlerFn& fn) {
    auto wl = lock_.write_lock();
    handlers_.emplace_back(EXCEPTION_ACCESS_VIOLATION, address, fn);

    if (handlers_.size() == 1) {
      auto self = plx::Globals::get<VEHManager>();
      if (!self)
        throw 7;  // $$$ fix.
      veh_handler_ = ::AddVectoredExceptionHandler(1, &VecHandler);
    }
  }

  void remove_av_handler(const plx::Range<uint8_t>& address) {
    auto wl = lock_.write_lock();
    for (auto it = cbegin(handlers_); it != cend(handlers_); ++it) {
      if (it->address.start() != address.start())
        continue;
      if (it->code == EXCEPTION_ACCESS_VIOLATION) {
        handlers_.erase(it);
        if (handlers_.size() == 0)
          ::RemoveVectoredExceptionHandler(veh_handler_);
        return;
      }
    }
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

  File() = delete;
  File(const File&) = delete;
  File& operator=(const File&) = delete;

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

  size_t read(plx::Range<uint8_t>& mem, unsigned int from = -1) {
    return read(mem.start(), mem.size(), from);
  }

  size_t read(uint8_t* buf, size_t len, int from) {
    OVERLAPPED ov = {0};
    ov.Offset = from;
    DWORD read = 0;
    if (!::ReadFile(handle_, buf, static_cast<DWORD>(len),
                    &read, (from < 0) ? nullptr : &ov))
      return 0;
    return read;
  }

  size_t write(const plx::Range<const uint8_t>& mem, int from = -1) {
    return write(mem.start(), mem.size(), from);
  }

  size_t write(const uint8_t* buf, size_t len, int from) {
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
// plx::CmdLine (handles command line arguments)
//

class CmdLine {

  struct KeyHash {
    size_t operator()(const plx::Range<const wchar_t>& r) const {
      return plx::Hash_FNV1a_64(r.const_bytes());
    }
  };

  struct KeyEqual {
    bool operator()(const plx::Range<const wchar_t>& lhs, const plx::Range<const wchar_t>& rhs) const {
      return lhs.equals(rhs);
    }
  };

  std::unordered_map<plx::Range<const wchar_t>, plx::Range<const wchar_t>, KeyHash, KeyEqual> opts_;
  std::vector<plx::Range<const wchar_t>> extra_;
  plx::Range<const wchar_t> program_;

public:
  CmdLine(int argc, wchar_t* argv[]) {
    if (!argc)
      return;

    int start;
    auto arg0 = plx::RangeUntilValue<wchar_t>(argv[0], 0);
    if (is_program(arg0)) {
      program_ = arg0;
      start = 1;
    } else {
      start = 0;
    }

    for (int ix = start; ix != argc; ++ix) {
      auto c_arg = plx::RangeUntilValue<wchar_t>(argv[ix], 0);
      if (IsOption(c_arg)) {
        c_arg.advance(2);
        opts_.insert(NameValue(c_arg));
      } else {
        extra_.push_back(c_arg);
      }
    }
  }

  template <size_t count>
  const bool has_switch(const wchar_t (&str)[count],
                        plx::Range<const wchar_t>* value = nullptr) const {

    return has_switch(plx::RangeFromLitStr<const wchar_t, count>(str), value);
  }

  const bool has_switch(const plx::Range<const wchar_t>& name,
                        plx::Range<const wchar_t>* value = nullptr) const {
    auto pos = opts_.find(name);
    bool found = pos != end(opts_);
    if (value && found) {
      *value = pos->second;
      return true;
    }
    return found;
  }

  size_t extra_count() const {
    return extra_.size();
  }

  plx::Range<const wchar_t> extra(size_t index) const {
    if (index >= extra_.size())
      return plx::Range<wchar_t>();
    return extra_[index];
  }

private:
  bool IsOption(const plx::Range<wchar_t>& r) {
    if (r.size() < 3)
      return false;
    return ((r[0] == '-') && (r[1] == '-') && (r[2] != '-') && (r[2] != '='));
  }

  bool is_program(const plx::Range<wchar_t>& r) const {
    // $$$ todo.
    return false;
  }

  std::pair<plx::Range<wchar_t>, plx::Range<wchar_t>> NameValue(plx::Range<wchar_t>& r) {
    size_t pos = 0;
    if (r.contains(L'=', &pos) && pos < (r.size() - 1))
      return std::make_pair(
          plx::Range<wchar_t>(r.start(), r.start() + pos),
          plx::Range<wchar_t>(r.start() + pos + 1, r.end()));
    return std::make_pair(r, plx::Range<wchar_t>());
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::JsonException
//
class JsonException : public plx::Exception {

public:
  JsonException(int line)
      : Exception(line, "Json exception") {
    PostCtor();
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
  typedef ObjectImpl::const_iterator KeyValueIterator;

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

  bool has_key(const std::string& k) const {
    return (GetObject()->find(k) != end(*GetObject()));
  }

  std::pair<KeyValueIterator, KeyValueIterator>  get_iterator() const {
    return std::make_pair(GetObject()->begin(), GetObject()->end());
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
      throw plx::JsonException(__LINE__);
    void* addr = &u_.obj;
    return reinterpret_cast<ObjectImpl*>(addr);
  }

  const ObjectImpl* GetObject() const {
    if (type_ != JsonType::OBJECT)
      throw plx::JsonException(__LINE__);
    const void* addr = &u_.obj;
    return reinterpret_cast<const ObjectImpl*>(addr);
  }

  ArrayImpl* GetArray() {
    if (type_ != JsonType::ARRAY)
      throw plx::JsonException(__LINE__);
    void* addr = &u_.arr;
    return reinterpret_cast<ArrayImpl*>(addr);
  }

  const ArrayImpl* GetArray() const {
    if (type_ != JsonType::ARRAY)
      throw plx::JsonException(__LINE__);
    const void* addr = &u_.arr;
    return reinterpret_cast<const ArrayImpl*>(addr);
  }

  std::string* GetString() {
    if (type_ != JsonType::STRING)
      throw plx::JsonException(__LINE__);
    void* addr = &u_.str;
    return reinterpret_cast<StringImpl*>(addr);
  }

  const std::string* GetString() const {
    if (type_ != JsonType::STRING)
      throw plx::JsonException(__LINE__);
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
// plx::DemandPagedMemory
//
class DemandPagedMemory {
  size_t page_faults_;
  size_t block_size_;
  plx::Range<uint8_t> range_;

public:
  DemandPagedMemory(size_t max_bytes, size_t block_pages)
      : page_faults_(0),
        block_size_(block_pages * 4096),
        range_(reinterpret_cast<uint8_t*>(
            ::VirtualAlloc(NULL, max_bytes, MEM_RESERVE, PAGE_NOACCESS)), max_bytes) {
    if (!range_.start())
      throw plx::MemoryException(__LINE__, 0);
    auto veh_man = plx::Globals::get<plx::VEHManager>();
    plx::VEHManager::HandlerFn fn =
        std::bind(&DemandPagedMemory::page_fault, this, std::placeholders::_1);
    veh_man->add_av_handler(range_, fn);
  }

  ~DemandPagedMemory() {
    auto veh_man = plx::Globals::get<plx::VEHManager>();
    veh_man->remove_av_handler(range_);
    ::VirtualFree(range_.start(), 0, MEM_RELEASE);
  }

  plx::Range<uint8_t> get() { return range_; }

  size_t page_faults() const { return page_faults_; }

private:
  bool page_fault(EXCEPTION_RECORD* er) {
    ++page_faults_;
    auto addr = reinterpret_cast<uint8_t*>(er->ExceptionInformation[1]);
    auto alloc_sz = std::min(block_size_, size_t(range_.end() - addr));
    auto new_addr = ::VirtualAlloc(addr, alloc_sz, MEM_COMMIT, PAGE_READWRITE);
    return (new_addr != nullptr);
  }
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
// plx::HuffmanCodec : implements a huffman decoder.
// it maps bit patterns (codes) of up to 15 bits to 16 bit symbols.
//
// |lengths| is the number of bits each symbol should use. With 0 bits meaning
// that the symbol does not get a code. Based on that a huffman decoder zlib
// compatible can be constructed like RFC1951 documents.
//

class HuffmanCodec {
  std::vector<uint16_t> symbols_;
  std::vector<uint16_t> counts_;

public:
  HuffmanCodec(size_t max_bits, const plx::Range<uint16_t>& lengths) {
    if (max_bits > 15)
      throw plx::InvalidParamException(__LINE__, 1);

    counts_.resize(max_bits + 1);
    for (auto len : lengths) {
      counts_[len]++;
    }

    // all codes taking 0 bits makes no sense.
    if (counts_[0] == lengths.size())
      throw plx::InvalidParamException(__LINE__, 2);

    // compute how many symbols are not coded. |left| < 0 means the
    // code is over-subscribed (error), |left| == 0 means every
    // code maps to a symbol.
    int left = 1;
    for (size_t bit_len = 1; bit_len <= max_bits; ++bit_len) {
      left *= 2;
      left -= counts_[bit_len];
      if (left < 0)
        throw plx::InvalidParamException(__LINE__, 2);
    }

    // Generate offsets for each bit lenght.
    std::vector<uint16_t> offsets;
    offsets.resize(max_bits + 1);
    offsets[0] = 0;
    for (size_t len = 1; len != max_bits; ++len) {
      offsets[len + 1] = offsets[len] + counts_[len];
    }

    symbols_.resize(lengths.size());
    uint16_t symbol = 0;
    for (auto losym : lengths) {
      if (losym)
        symbols_[offsets[losym]++] = symbol;
      ++symbol;
    }
  }

  int decode(plx::BitSlicer& slicer) {
    const size_t max_bits = counts_.size() - 1;
    uint16_t code = 0;
    uint16_t first = 0;
    size_t index = 0;
    // $$ todo, replace for an efficient table lookup.
    for (size_t len = 1; len <= max_bits; ++len) {
      code |= slicer.slice(1);
      auto count = counts_[len];
      auto d = code - first;
      if (count > d )
        return symbols_[index + d];
      index += count;
      first += count;
      first *= 2;
      code <<= 1;
    }
    return -1;
  }

};


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

char32_t DecodeUTF8(plx::Range<const uint8_t>& ir) ;


///////////////////////////////////////////////////////////////////////////////
// plx::Inflater  : Implements a decompressor for the DEFLATE format following
//                  RFC 1951. It takes a compressed byte range |r| and returns
//                  via output() the decompressed data.
class Inflater {
  int error_;
  std::unique_ptr<plx::DemandPagedMemory> dpm_;
  plx::Range<uint8_t> output_;

  std::unique_ptr<plx::HuffmanCodec> liter_len_;
  std::unique_ptr<plx::HuffmanCodec> distance_;

public:
  enum Errors {
    success = 0,
    not_implemented,
    invalid_block_type,
    invalid_length,
    empty_block,
    missing_data,
    invalid_symbol,
    bad_dynamic_dict,
    too_many_lengths,
    bad_huffman
  };

  Inflater() : error_(success)  {
  }

  Inflater(const Inflater&) = delete;
  Inflater& operator=(const Inflater&) = delete;

  const plx::Range<uint8_t>& output() const {
    return output_;
  }

  Errors status() const { return Errors(error_); }

  size_t inflate(const plx::Range<const uint8_t>& r) {
    if (r.empty()) {
      error_ = empty_block;
      return 0;
    }

    dpm_.reset(new DemandPagedMemory(r.size(), 4));
    output_ = plx::Range<uint8_t>(dpm_->get().start(), size_t(0));

    error_ = success;
    plx::BitSlicer slicer(r);

    while (true) {
      auto last = slicer.slice(1) == 1;
      auto type = slicer.slice(2);

      switch (type) {
      case 0:  error_ = stored(slicer);
        break;
      case 1:  error_ = fixed(slicer);
        break;
      case 2:  error_ = dynamic(slicer);
        break;
      default:
        error_ = invalid_block_type;
        break;
      }

      if (error_) {
        break;
      } else if (last) {
        break;
      } else if (slicer.past_end()) {
        error_ = missing_data;
        break;
      }
    }

    return (error_ == success) ? slicer.pos() : 0;
  }

  Errors stored(plx::BitSlicer& slicer) {
    // The size of the block comes next in two bytes and its complement.
    slicer.discard_bits();
    unsigned int len = slicer.slice(16);
    unsigned int clen = slicer.slice(16);
    // check complement.
    if (len != ((~clen) & 0x0ffff))
      return invalid_length;

    if (!len)
      return empty_block;

    // copy |len| bytes to the output.
    const auto& remaining = slicer.remaining_range();
    if (remaining.size() < len)
      return missing_data;

    memcpy(output_.end(), remaining.start(), len);
    output_.extend(len);

    slicer.skip(len);
    return success;
  }

  Errors fixed(plx::BitSlicer& slicer) {
    construct_fixed_codecs();
    return decode(slicer, *liter_len_, *distance_);
  }

  Errors dynamic(plx::BitSlicer& slicer) {
    // number of Literal & Length codes, from 257 to 286.
    auto ll_len = slicer.slice(5) + 257;
    // number of distance codes, from 1 to 32.
    auto dist_len = slicer.slice(5) + 1;
    // number of code length codes, from 4 to 19.
    auto clc_len = slicer.slice(4) + 4;

    if (ll_len > 286)
      return bad_dynamic_dict;
    if (dist_len > 30)
      return bad_dynamic_dict;

    // the |order| represents the order most common symbols expected from the
    // next decoding phase: 16 = rle case 1, 17 = rle case 2 and 18 = case 3,
    // then 0 = no code assigned. The rest are bit lenghts.
    static const size_t order[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    // Build the huffman decoder for the code length codes.
    std::vector<uint16_t> lengths;
    lengths.resize(19);
    size_t index = 0;
    for (; index != clc_len; ++index)
      lengths[order[index]] = static_cast<uint16_t>(slicer.slice(3));
    for (; index < 19; ++index)
      lengths[order[index]] = 0;
    plx::HuffmanCodec clc_huffman(15, plx::RangeFromVector(lengths));

    // Now read the real huffman decoder for the dynamic block.
    const auto table_len = ll_len + dist_len;

    lengths.resize(table_len);
    size_t ix = 0;

    while (ix < table_len) {
      auto symbol = clc_huffman.decode(slicer);
      if (symbol < 0)
        return invalid_symbol;
      if (symbol < 16) {
        // the symbol is the huffman bit lenght.
        lengths[ix++] = static_cast<uint16_t>(symbol);
      } else {
        // run length encoded. There are 3 cases.
        int rle_value;
        int rle_count;

        if (symbol == 16) {
          if (!ix)
            return bad_dynamic_dict;
          // case 1: repeat last bit length value 3 to 6 times.
          rle_value = lengths[ix - 1];
          rle_count = 3 + slicer.slice(2);
        } else {
          rle_value = 0;
          if (symbol == 17) {
            // case 2: 0 bit length (no code for symbol) is repeated 3 to 10 times.
            rle_count = 3 + slicer.slice(3);
          } else {
            // case 3: 0 bit length (no code for symbol) is repeated 11 to 138 times.
            rle_count = 11 + slicer.slice(7);
          }
        }

        if (ix + rle_count > table_len)
          return too_many_lengths;

        while (rle_count--)
          lengths[ix++] = static_cast<uint16_t>(rle_value);
      }
    }

    // we should have received a non-zero bit length for code 256, because
    // that is how the decompressor will know how to stop.
    if (!lengths[256])
      return bad_huffman;

    // Finally construct the two huffman decoders and decode the stream.
    auto rfv = plx::RangeFromVector(lengths);
    plx::HuffmanCodec lit_len(15, rfv.slice(0, ll_len));
    plx::HuffmanCodec distance(15, rfv.slice(ll_len));

    return decode(slicer, lit_len, distance);
  }

  void construct_fixed_codecs() {
    if (liter_len_ && distance_)
      return;

    const size_t fixed_ll_codes = 288;
    const size_t fixed_dis_codes = 30;

    std::vector<uint16_t> lengths;
    lengths.resize(fixed_ll_codes);
    size_t symbol;

    for (symbol = 0; symbol != 144; ++symbol)
      lengths[symbol] = 8;
    for (; symbol != 256; ++symbol)
      lengths[symbol] = 9;
    for (; symbol != 280; ++symbol)
      lengths[symbol] = 7;
    for (; symbol != fixed_ll_codes; ++symbol)
      lengths[symbol] = 8;

    // Symbol 286 and 287 should never appear in the stream.
    liter_len_.reset(new plx::HuffmanCodec(15, plx::RangeFromVector(lengths)));

    lengths.resize(fixed_dis_codes);
    for (symbol = 0; symbol != fixed_dis_codes; ++symbol)
      lengths[symbol] = 5;

    distance_.reset(new plx::HuffmanCodec(15, plx::RangeFromVector(lengths)));
  }

  Errors decode(plx::BitSlicer& slicer, plx::HuffmanCodec& len_lit, plx::HuffmanCodec& dist) {
    while (true) {
      // either a literal or the length of a <len, dist> pair.
      auto symbol = len_lit.decode(slicer);
      if (symbol < 0)
        return invalid_symbol;

      if (symbol == 256) {
        // special symbol that signifies the end of stream.
        return success;
      } else if (symbol < 256) {
        // literal.
        *output_.end() = symbol;
        output_.extend(1);
      } else {
        // above 256 means length - distance pair.
        auto len = decode_length(symbol, slicer);
        if (len < 0)
          return invalid_symbol;
        symbol = dist.decode(slicer);
        if (symbol < 0)
          return invalid_symbol;
        auto dist = decode_distance(symbol, slicer);
        // copy data from the already decoded stream.
        back_copy(dist, len);
      }
    }  // while.
  }

  //$$$ todo: remove the static arrays below, they are not thread safe.

  int decode_length(int symbol, plx::BitSlicer& slicer) {
    static const int lens[29] = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
    };
    static const int lext[29] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
    };

    symbol -= 257;
    if (symbol >= 29)
      return -1;

    auto extra_bits = lext[symbol];
    return lens[symbol] +  (extra_bits ? slicer.slice(extra_bits) : 0);
  }

  int decode_distance(int symbol, plx::BitSlicer& slicer) {
    static const int dists[30] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577
    };

    static const short dext[30] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
    };

    if (symbol >= 29)
      return -1;

    auto extra_bits = dext[symbol];
    return dists[symbol] + (extra_bits ? slicer.slice(extra_bits) : 0);
  }

  void back_copy(int from, int count) {
    auto s = output_.size() - from;
    for (int ix = 0; ix != count; ++ix) {
      *output_.end() = output_[s++];
      output_.extend(1);
    }
  }

};


///////////////////////////////////////////////////////////////////////////////
// plx::SizeL : windows compatible SIZE wrapper.
//

struct SizeL : public ::SIZE {
  SizeL() : SIZE() {}
  SizeL(long width, long height) : SIZE({width, height}) {}
  bool empty() const { return (cx == 0L) && (cy == 0L); }
};


///////////////////////////////////////////////////////////////////////////////
// plx::RectL : windows compatible RECT wrapper.
//

struct RectL : public ::RECT {
  RectL() : RECT() {}
  RectL(long x0, long y0, long x1, long y1) : RECT({x0, y0, x1, y1}) {}
  RectL(const plx::SizeL& size): RECT({ 0L, 0L, size.cx, size.cy }) {}

  plx::SizeL size() { return SizeL(right - left, bottom - top); }
  long width() const { return right - left; }
  long height() const { return bottom - top; }
};


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

private:
  FilesInfo() : info_(nullptr), done_(false) {}
  FilesInfo(const FilesInfo&) = delete;

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
};


///////////////////////////////////////////////////////////////////////////////
// plx::UTF16FromUTF8
std::wstring UTF16FromUTF8(const plx::Range<const uint8_t>& utf8) ;


///////////////////////////////////////////////////////////////////////////////
// plx::UTF8FromUTF16
std::string UTF8FromUTF16(const plx::Range<const uint16_t>& utf16) ;


///////////////////////////////////////////////////////////////////////////////
// plx::GZip  : implements a GZip compatible decompressor.
//

class GZip {
  plx::Inflater inflater_;
  std::string fname_;

public:
  GZip() {
  }

  plx::Range<const uint8_t> output() const {
    return inflater_.output();
  }

  const std::string& file_name() const {
    return fname_;
  }

  bool extract(plx::Range<const uint8_t>& input) {
    plx::BitSlicer slicer(input);
    auto magic = slicer.slice(16);
    if (magic != 0x8b1f)
      return false;
    auto method = slicer.slice(8);
    if (method != 8)
      return false;

    // Flags:
    // bit 0  FTEXT : text hint. Don't care.
    // bit 1  FHCRC : not checked.
    // bit 2  FEXTRA : skipped.
    // bit 3  FNAME : skipped ($$ todo fix).
    // bit 4  FCOMMENT : skipped.
    slicer.slice(1);
    bool has_crc = slicer.slice(1) == 1;
    bool has_extra = slicer.slice(1) == 1;
    bool has_name = slicer.slice(1) == 1;
    bool has_comment = slicer.slice(1) == 1;
    auto reserved = slicer.slice(3);

    if (reserved)
      return false;

    auto mtime = slicer.next_uint32();
    auto xfl = slicer.slice(8);
    auto os = slicer.slice(8);

    if (has_extra) {
      auto xlen = slicer.slice(16);
      slicer.skip(xlen);
    }

    slicer.discard_bits();

    if (has_name) {
      auto name = slicer.remaining_range();
      size_t pos = 0;
      if (!name.contains(0, &pos))
        return false;
      fname_ = reinterpret_cast<const char*>(name.start());
      slicer.skip(pos + 1);
    }

    if (has_comment) {
      auto comment = slicer.remaining_range();
      size_t pos = 0;
      if (!comment.contains(0, &pos))
        return false;
      slicer.skip(pos + 1);
    }

    if (has_crc) {
      auto crc = slicer.slice(16);
    }

    slicer.discard_bits();

    auto consumed = inflater_.inflate(slicer.remaining_range());
    if (!consumed)
      return false;

    slicer.skip(consumed);
    auto crc = slicer.next_uint32();
    // $$$ todo: chec crc.
    auto isize = slicer.next_uint32();

    if (isize != inflater_.output().size())
      return false;

    input.advance(consumed);
    return true;
  }

};

}

extern "C" IMAGE_DOS_HEADER __ImageBase;
