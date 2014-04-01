// ut_core.cpp
#define NOMINMAX

#include "../utests.h"

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
  plx::JsonValue obj1(plx::JsonType::OBJECT);
  obj1["foo"] = value;
  obj1["bar"] = plx::JsonValue("aruba");
  plx::JsonValue obj2(obj1);
  obj2["sun"] = "screen";
  obj2["lava"] = 333;
  plx::JsonValue obj3 = {22, 34, 77, 11, 55};
  plx::JsonValue obj4 = { "sun", 12, false, "rum", obj2 };

}
