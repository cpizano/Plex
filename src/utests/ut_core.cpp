// ut_core.cpp
#define NOMINMAX

#include "../utests.h"

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
  auto x = plx::To<int>(2);
  CheckEQ(x, 2);
  auto y = plx::To<int>(short(3));
  CheckEQ(y, 3);
  auto z = plx::To<int>(char('a'));
  CheckEQ(z, 'a');

  auto a = plx::To<int>(float(100));
  CheckEQ(a, 100);
  auto b = plx::To<char>(float(127));
  CheckEQ(b, char(127));
  auto c = plx::To<char>(float(-127));
  CheckEQ(c, char(-127));

  try {
    auto d = plx::To<char>(float(128));
    NotReached(d);
  } catch (plx::OverflowException& ex) {
    CheckEQ(ex.kind(), plx::OverflowKind::Positive);
  }

  try {
    auto e = plx::To<char>(float(-129));
    NotReached(e);
  } catch (plx::OverflowException& ex) {
    CheckEQ(ex.kind(), plx::OverflowKind::Negative);
  }

}

