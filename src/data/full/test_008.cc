// test_008, part of the plex test suite.

#include <windows.h>
#include <intrin.h>
#include <stdio.h>

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
}

typedef long CustomType;

int wmain(int argc, wchar_t* argv[]) {
  if (argc > 2)
    throw plx::Exception(__LINE__, "missing args");

  auto fnn = [] (int val) -> CustomType {
    return CustomType(val);
  };

  plx::CpuId cpu_id;
  printf("= stepping %d model %d family %d\n",
         cpu_id.stepping(), cpu_id.model(), cpu_id.family());
  return 0;
}
