// ut_core.cpp
#define NOMINMAX

#include "../utests.h"




#include <windows.h>
#include <intrin.h>
#include <array>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <utility>
#include <limits>
#include <type_traits>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// plx::Exception
// line_ : The line of code, usually __LINE__.
// message_ : Whatever useful text.
namespace plx {
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
    return s_[0];
  }

  RefT back() const {
    return e_[-1];
  }

  RefT operator[](size_t i) const {
    return s_[i];
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

  bool advance(size_t count) {
    s_ += count;
    return (s_ < e_);
  }

  void clear() {
    s_ = It();
    e_ = It();
  }

};

///////////////////////////////////////////////////////////////////////////////
// plx::Range
template <typename T>
using Range = plx::ItRange<T*>;

///////////////////////////////////////////////////////////////////////////////
// plx::CodecException
// bytes_ : The 16 bytes or less that caused the issue.
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

///////////////////////////////////////////////////////////////////////////////
// plx::JsonValue
//
template <typename T> using AligedStore =
    std::aligned_storage<sizeof(T), __alignof(T)>;


enum class JsonType {
  NULLT,
  BOOL,
  INT64,
  DOUBLE,
  ARRAY,
  OBJECT,
  STRING,
};

struct JsonObject {
  static JsonObject Make() { return JsonObject(); }
};

class JsonValue {
  typedef std::unordered_map<std::string, JsonValue> ObjectImpl;
  typedef std::vector<JsonValue> ArrayImpl;
  typedef std::string StringImpl;

  JsonType type_;
  union Data {
    explicit Data() {}
    ~Data() {}

    void* nulv;
    bool bolv;
    double dblv;
    int64_t intv;
    AligedStore<StringImpl>::type str;
    AligedStore<ArrayImpl>::type arr;
    //AligedStore<ObjectImpl>::type obj;
    std::aligned_storage<sizeof(ObjectImpl), __alignof(ObjectImpl)>::type obj;
  } u_;

 public:

  JsonValue() : type_(JsonType::NULLT) {
  }

  ~JsonValue() {
    Destroy();
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

  JsonValue(const JsonObject&) : type_(JsonType::OBJECT) {
    new (&u_.obj) ObjectImpl();
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
    ObjectImpl& obj = *GetObject();
    return obj[s];
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
enum class OverflowKind {
  None,
  Positive,
  Negative,
};

///////////////////////////////////////////////////////////////////////////////
// plx::OverflowException
// kind_ : Type of overflow, positive or negative.
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
// plx::DecodeUTF8
//
// bits encoding
// 7    0xxxxxxx
// 11   110xxxxx 10xxxxxx
// 16   1110xxxx 10xxxxxx 10xxxxxx
// 21   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// The 5 and 6 bytes encoding are no longer valid since RFC 3629.
// max point is then U+10FFFF.
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
// plx::To  (integer to integer type safe cast)

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
  range3.advance(3);
  CheckEQ(range3.size(), range2.size() - 3);
  CheckEQ(range3.front(), 'd');
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
    const unsigned char t[] = "the lazy";
    plx::Range<const unsigned char> r(t, sizeof(t) - 1);
    std::u32string result;
    while (r.size()) {
      auto c = plx::DecodeUTF8(r);
      result.push_back(c);
    }
    std::u32string expected = { 't','h','e',' ','l','a','z','y' };
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
    // unicode U+2260 (not equal to) is 0xE2 0x89 0xA0
    const unsigned char t[] = {0xE2, 0x89, 0xA0};
    plx::Range<const unsigned char> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0x2260);
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

  // overlong forms for U+000A (line feed)
  //   0xC0 0x8A
  //   0xE0 0x80 0x8A
  //   0xF0 0x80 0x80 0x8A
  //   0xF8 0x80 0x80 0x80 0x8A
  //   0xFC 0x80 0x80 0x80 0x80 0x8A
}


void Test_JsonValue::Exec() {
  plx::JsonValue value("most likely");
  plx::JsonValue obj(plx::JsonObject::Make());
  obj["foo"] = value;
  obj["bar"] = plx::JsonValue("aruba");


}
