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

