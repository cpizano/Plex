// ut_core.cpp
#define NOMINMAX

#include "../utests.h"




#include <windows.h>
#include <intrin.h>
#include <iterator>
#include <limits>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////
// plx::CpuId
// id_ : the four integers returned by the 'cpuid' instruction.
namespace plx {
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
// plx::Exception
// line_ : The line of code, usually __LINE__.
// message_ : Whatever useful text.
class Exception {
  int line_;
  const char* message_;

protected:
  void PostCtor() {
    if (::IsDebuggerPresent()) {
      __debugbreak();
    }
  }

public:
  Exception(int line, const char* message) : line_(line), message_(message) {}
  virtual ~Exception() {}
  const char* Message() const { return message_; }
  int Line() const { return line_; }
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

  void clear() {
    s_ = It();
    e_ = It();
  }

  ValueT& front() {
    return s_[0];
  }

  ValueT& back() {
    return e_[-1];
  }

  const ValueT& front() const {
    return s_[0];
  }

  const ValueT& back() const {
    return e_[-1];
  }

  ValueT& operator[](size_t i) {
    return s_[i];
  }

  const ValueT& operator[](size_t ix) const {
    return s_[ix];
  }

};

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
// plx::Range
template <typename T>
using Range = plx::ItRange<T*>;

///////////////////////////////////////////////////////////////////////////////
// plx::To  (integer to integer type safe cast)
template <typename Tgt, typename Src>
Tgt To(const Src & value) {
  if (std::numeric_limits<Tgt>::max() < std::numeric_limits<Src>::max())
    throw plx::OverflowException(__LINE__, OverflowKind::Positive);

  if (std::numeric_limits<Src>::is_signed) {
    if (!std::numeric_limits<Tgt>::is_signed)
      throw plx::OverflowException(__LINE__, OverflowKind::Negative);
    if (sizeof(Src) > sizeof(Tgt))
      throw plx::OverflowException(__LINE__, OverflowKind::Negative);
  }

  return static_cast<Tgt>(value);
}
}

void Test_Range::Exec() {
  // Empty iterator is empty.
  plx::Range<const char> range1;
  CheckEQ(range1.empty(), true);
  CheckEQ(range1.size(), size_t(0));

  // The range so defined includes the null character.
  const char txt1[] = "abcdefghijklmnopqrstuv";
  const plx::Range<const char> range2(txt1, sizeof(txt1));
  CheckEQ(range2.empty(), false);
  CheckEQ(range2.size(), sizeof(txt1));
  CheckEQ(range2.front(), 'a');
  CheckEQ(range2.back(), '\0');
  CheckEQ(range2[2], 'c');
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

void Test_To_Integer::Exec() {
  int x = plx::To<int>(2);
  int y = plx::To<int>(short(3));
  int z = plx::To<int>(char('a'));
  CheckEQ(x, 2);
  CheckEQ(y, 3);
  CheckEQ(z, 'a');
}
