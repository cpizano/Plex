// ut_core.cpp
#include "stdafx.h"
#include "utests.h"

void Test_PlatformCheck::Exec() {
  CheckEQ(plx::PlatformCheck(), true);

  auto luid = plx::LocalUniqueId();
  CheckEQ(luid != 0, true);
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

  uint8_t data[] = {0x11, 0x22, 0x33};
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

  {
    plx::Range<char> r(0, 120);
    auto mem = plx::HeapRange(r);
    r[0] = 22;
    r[1] = 33;
    CheckEQ(r.start() != 0, true);
    CheckEQ(r.size(), 120);
  }

  {
    plx::Range<char> r(0, 5);
    plx::Range<const char> cr(r);
    CheckEQ(cr.size(), 5);
  }

  {
    auto r1 = plx::RangeUntilValue<const char>("rez,pez", ',');
    CheckEQ(r1.equals(plx::RangeFromLitStr("rez")), true);
  }

  {
    std::vector<long> v = {4, 5, 6};
    auto r1 = plx::RangeFromVector(v);
    CheckEQ(r1[1], 5);
    CheckEQ(r1.size(), 3);

    std::vector<long> q;
    for (auto x : r1) {
      q.push_back(x);
    }

    CheckEQ(v, q);
  }

}

void Test_BitSlice::Exec() {
  const uint8_t tv[] = {
      // lsb - msb lsb - msb
      // 0110 1001 0100 0100 1110 0001 0010 1000 1111 1110
      // --+--+------+-------------------++------+----+
      0x96, 0x22, 0x87, 0x14, 0x7f
  };

  auto r = plx::RangeFromArray(tv);
  plx::BitSlicer slicer(r);
  CheckEQ(slicer.past_end(), false);

  auto s1 = slicer.slice(3);
  CheckEQ(s1 == 0x06, true);
  CheckEQ(slicer.past_end(), false);
  auto s2 = slicer.slice(2);
  CheckEQ(s2 == 0x02, true);
  CheckEQ(slicer.past_end(), false);
  auto s3 = slicer.slice(6);
  CheckEQ(s3 == 0x14, true);
  CheckEQ(slicer.past_end(), false);
  auto s4 = slicer.slice(16);
  CheckEQ(s4 == 0x90e4, true);
  CheckEQ(slicer.past_end(), false);
  auto s5 = slicer.slice(1);
  CheckEQ(s5 == 0x00, true);
  CheckEQ(slicer.past_end(), false);
  auto s6 = slicer.slice(5);
  CheckEQ(s6 == 0x11, true);
  auto s7 = slicer.slice(4);
  CheckEQ(s7 == 0x0f, true);
  auto s8 = slicer.slice(1);
  CheckEQ(s8 == 0x01, true);
  bool end = false;
  auto s9 = slicer.slice(5, &end);
  CheckEQ(end, true);
  CheckEQ(slicer.past_end(), true);
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

void Test_LinkedBuffers::Exec() {
  plx::LinkedBuffers lb;
  auto r1 = lb.new_buffer(40);
  CheckEQ(r1.size(), 40UL);
  auto r2 = lb.new_buffer(50);
  CheckEQ(r2.size(), 50UL);
  auto r3 = lb.new_buffer(60);
  CheckEQ(r3.size(), 60UL);
  r1[0] = 'a';
  r2[0] = 'b';
  r3[0] = 'c';
  int ix = 0;
  for (lb.first(); !lb.done(); lb.next()) {
    CheckEQ(lb.get()[0], 'a' + ix);
    ++ix;
  }
  CheckEQ(ix, 3);
  int iz = 0;
  plx::LinkedBuffers lb2 = lb;
  for (lb2.first(); !lb2.done(); lb2.next()) {
    CheckEQ(lb2.get()[0], 'a' + iz);
    ++iz;
  }
  CheckEQ(iz, 3);
  plx::LinkedBuffers lb3(std::move(lb2));
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
    const uint8_t t[] = "\0";
    plx::Range<const uint8_t> r(t, 1);
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0);
  }

  {
    const uint8_t t[] = "the lazy.";
    plx::Range<const uint8_t> r(t, sizeof(t) - 1);
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
    const uint8_t t[] = {0xC2, 0x80};
    plx::Range<const uint8_t> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0x80);
  }

  {
    // unicode U+00A9 (copyright sign) is 0xC2 0xA9
    const uint8_t t[] = {0xC2, 0xA9};
    plx::Range<const uint8_t> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0xA9);
  }

  {
    // first tree byte code U+0800 (samaritan alaf) is 
    const uint8_t t[] = {0xE0, 0xA0, 0x80};
    plx::Range<const uint8_t> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0x800);
  }

  {
    // last 3 byte code U+FFFF (non character) is 0xE2 0x89 0xA0
    const uint8_t t[] = {0xEF, 0xBF, 0xBF};
    plx::Range<const uint8_t> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0xFFFF);
  }

  {
    // unicode U+2260 (not equal to) 0xE2 0x89 0xA0
    const uint8_t t[] = {0xE2, 0x89, 0xA0};
    plx::Range<const uint8_t> r(t, sizeof(t));
    auto c = plx::DecodeUTF8(r);
    CheckEQ(c, 0x2260);
  }

  {
    // greek word 'kosme'.
    const uint8_t t[] =
        {0xCE,0xBA,0xE1,0xBD,0xB9,0xCF,0x83,0xCE,0xBC,0xCE,0xB5};
    plx::Range<const uint8_t> r(t, sizeof(t));
    std::u32string result;
    while (r.size()) {
      auto c = plx::DecodeUTF8(r);
      result.push_back(c);
    }
    CheckEQ(result.size() == 5, true);
  }

   {
    // overlong form #1 for U+000A (line feed) 0xC0 0x8A
    const uint8_t t[] = {0xC0, 0x8A};
    plx::Range<const uint8_t> r(t, sizeof(t));
    try {
      auto c = plx::DecodeUTF8(r);
      __debugbreak();
    } catch (plx::CodecException& e) {
      CheckEQ(e.bytes(), "C0,8A");
    }
  }

  {
    // overlong form #2 for U+000A (line feed) 0xE0 0x80 0x8A
    const uint8_t t[] = {0xE0, 0x80, 0x8A};
    plx::Range<const uint8_t> r(t, sizeof(t));
    try {
      auto c = plx::DecodeUTF8(r);
      __debugbreak();
    } catch (plx::CodecException& e) {
      CheckEQ(e.bytes(), "E0,80,8A");
    }
  }

  {
    // overlong form #3 for U+000A (line feed) 0xF0 0x80 0x80 0x8A
    const uint8_t t[] = {0xF0, 0x80, 0x80, 0x8A};
    plx::Range<const uint8_t> r(t, sizeof(t));
    try {
      auto c = plx::DecodeUTF8(r);
      __debugbreak();
    } catch (plx::CodecException& e) {
      CheckEQ(e.bytes(), "F0,80,80,8A");
    }
  }

  {
    // overlong form #4 for U+000A (line feed) 0xF8 0x80 0x80 0x80 0x8A
    const uint8_t t[] = {0xF8, 0x80, 0x80, 0x80, 0x8A};
    plx::Range<const uint8_t> r(t, sizeof(t));
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

void Test_StringPrintf::Exec() {
  auto s1 = plx::StringPrintf("the %s jumped over the %d lazy %s", "fox", 3, "dogs");
  CheckEQ(s1 == "the fox jumped over the 3 lazy dogs", true);

  int n = 1073741824;
  auto s2 = plx::StringPrintf(
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
      n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n);
  CheckEQ(s2.size(), 220);
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

    int count = 0;
    auto it = value.get_iterator();
    while (it.first != it.second) {
      ++count;
      ++it.first;
    }
    CheckEQ(count, 5);

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
    uint8_t ts[] = "123456789";
    CheckEQ(plx::CRC32C(0, ts, sizeof(ts) - 1), 0xE3069283);
  }
  {
    uint8_t ts[] = " the lazy fox jumps over";
    CheckEQ(plx::CRC32C(0, ts + 1, sizeof(ts) - 2), 0xEFB0976C);
  }
  {
    uint8_t ts[] = "0";
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
  {
    plx::FilePath fp1(L"data\\file_io\\data_001.txt");
    plx::File f = plx::File::Create(fp1, par3, plx::FileSecurity());
    CheckEQ(f.is_valid(), true);
    CheckEQ(f.status(), plx::File::existing);
    CheckEQ(f.size_in_bytes(), 2048UL);
  }
  {
    plx::FilePath fp1(L"c:\\windows\\system32");
    auto par = plx::FileParams::Directory_ShareAll();
    plx::File f = plx::File::Create(fp1, par, plx::FileSecurity());
    CheckEQ(f.is_valid(), true);
    CheckEQ(f.status(), plx::File::directory | plx::File::existing);

    plx::FilesInfo finf = plx::FilesInfo::FromDir(f, 100);
    int count_files = 0;
    int count_dirs = 0;
    for (finf.first(); !finf.done(); finf.next()) {
      auto name = finf.file_name();
      auto ctim = finf.creation_ns1600();
      CheckGT(name.size(), 0);
      CheckLT(name.size(), 70);
      CheckGT(ctim, 4096);
      if (finf.is_directory())
        ++count_dirs;
      else
        ++count_files;
    }
    CheckEQ((count_files > 3800) && (count_files < 4100), true);
    CheckEQ((count_dirs > 100) && (count_dirs < 120), true);
  }

  {
    auto fp = plx::GetExePath();
    auto par = plx::FileParams::Directory_ShareAll();
    plx::File f = plx::File::Create(fp, par, plx::FileSecurity());
    CheckEQ(f.status(), plx::File::directory | plx::File::existing);
  }

}

void Test_IOCPLoop::Exec() {
  struct Sum {
    int value;
    void Add(int addend) {
      value += addend;
    }
  };

  Sum sum = {55};
  plx::IOCPLoop loop;
  auto proxy = loop.MakeProxy();
  proxy.PostTask(std::bind(&Sum::Add, &sum, 5));
  proxy.PostTask(std::bind(&Sum::Add, &sum, 3));
  proxy.PostQuitTask();
  loop.Run(INFINITE);
  CheckEQ(sum.value, 63);
}

void Test_Hashes::Exec() {
  auto r1 = plx::RangeFromLitStr("foobar");
  auto r2 = plx::RangeFromLitStr("a");
  auto r3 = plx::RangeFromLitStr("");
  {
    auto hash1 = plx::Hash_FNV1a_64(r1.const_bytes());
    CheckEQ(hash1 == 0x85944171f73967e8ULL, true);
    auto hash2 = plx::Hash_FNV1a_64(r2.const_bytes());
    CheckEQ(hash2 == 0xaf63dc4c8601ec8cULL, true);
    auto hash3 = plx::Hash_FNV1a_64(r3.const_bytes());
    CheckEQ(hash3 == 0xcbf29ce484222325ULL, true);
  }
  {
    auto hash1 = plx::Hash_FNV1a_32(r1.const_bytes());
    CheckEQ(hash1 == 0xbf9cf968UL, true);
    auto hash2 = plx::Hash_FNV1a_32(r2.const_bytes());
    CheckEQ(hash2 == 0xe40c292cUL, true);
    auto hash3 = plx::Hash_FNV1a_32(r3.const_bytes());
    CheckEQ(hash3 == 0x811c9dc5UL, true);
  }
}

void Test_CmdLine::Exec() {
  wchar_t* args[] = {
    L"--verbose",
    L"sirens",
    L"--out=c:\\foo\\bar",
    L"-1",
    L"mermaids-one",
  };

  plx::CmdLine cmdline(5, args);

  CheckEQ(cmdline.extra_count() == 3, true);
  CheckEQ(cmdline.has_switch(L"verbose"), true);
  CheckEQ(cmdline.has_switch(L"sirens"), false);
  plx::Range<const wchar_t> out;
  CheckEQ(cmdline.has_switch(L"out", &out), true);
  CheckEQ(out.equals(plx::RangeFromLitStr(L"c:\\foo\\bar")), true);
}

void Test_Inflater::Exec() {
  {
    // A last block with invalid block type (11) 
    const uint8_t deflated_data[] = {
        0x07, 0x01, 0x00, 0xfe, 0xff, 0x55
    };

    plx::Inflater inflater;
    auto rv = inflater.inflate(plx::RangeFromArray(deflated_data));
    CheckEQ(rv, 0);
    CheckEQ(inflater.status(), plx::Inflater::invalid_block_type);
    CheckEQ(inflater.output().size(), 0); 
  }

  {
    // A last block with stored block (00) with 0 bytes.
    const uint8_t deflated_data[] = {
        0x01, 0x00, 0x00, 0xff, 0xff
    };

    plx::Inflater inflater;
    auto rv = inflater.inflate(plx::RangeFromArray(deflated_data));
    CheckEQ(rv, 0);
    CheckEQ(inflater.status(), plx::Inflater::empty_block);
    CheckEQ(inflater.output().size(), 0);
  }

  {
    // A stored block (00) with 1 byte. No final block.
    const uint8_t deflated_data[] = {
        0x00, 0x01, 0x00, 0xfe, 0xff, 0x55
    };

    plx::Inflater inflater;
    auto rv = inflater.inflate(plx::RangeFromArray(deflated_data));
    CheckEQ(rv, 0);
    CheckEQ(inflater.status(), plx::Inflater::missing_data);
    CheckEQ(inflater.output().size(), 1);
  }

  {
    // A stored block (00) with 1 byte, but it is missing.
    const uint8_t deflated_data[] = {
        0x00, 0x01, 0x00, 0xfe, 0xff
    };

    plx::Inflater inflater;
    auto rv = inflater.inflate(plx::RangeFromArray(deflated_data));
    CheckEQ(rv, 0);
    CheckEQ(inflater.status(), plx::Inflater::missing_data);
    CheckEQ(inflater.output().size(), 0);
  }

  {
    // 3 stored blocks (00) total of 8 bytes of payload.
    const uint8_t deflated_data[] = {
      0x00, 0x01, 0x00, 0xfe, 0xff, '0',
      0x00, 0x05, 0x00, 0xfa, 0xff, '1', '2', '3', '4', '5',
      0x01, 0x02, 0x00, 0xfd, 0xff, '6', '7',
    };

    plx::Inflater inflater;
    auto rv = inflater.inflate(plx::RangeFromArray(deflated_data));
    CheckEQ(rv, sizeof(deflated_data));
    CheckEQ(inflater.status(), plx::Inflater::success);
    CheckEQ(inflater.output().size(), 8);
  }
}

void Test_GZIP::Exec() {
  {
    // gzip stream with a single deflate (08) block.
    // deflate uses the fixed (1) dictionary.
    const uint8_t gzip_data[98] = {
        0x1f, 0x8b, 0x08, 0x08, 0x99, 0xf3, 0x15, 0x54,
        0x00, 0x0b, 0x30, 0x30, 0x31, 0x2e, 0x74, 0x78,
        0x74, 0x00, 0xcb, 0xc8, 0x2f, 0x2d, 0x4e, 0x55,
        0xc8, 0x05, 0x93, 0xc9, 0x89, 0x20, 0xd2, 0xd0,
        0xc8, 0xd8, 0xc4, 0x54, 0x21, 0xa5, 0xa8, 0x34,
        0x37, 0x37, 0xb5, 0x48, 0x01, 0xc4, 0x86, 0xc8,
        0xe6, 0xe7, 0x25, 0xa7, 0x2a, 0x94, 0x64, 0xa4,
        0x2a, 0x64, 0x80, 0xb9, 0x99, 0xc5, 0x60, 0x95,
        0x0a, 0xf9, 0x45, 0x38, 0x75, 0x94, 0x64, 0xe6,
        0x81, 0x45, 0x15, 0x80, 0x34, 0x42, 0x63, 0x7e,
        0x1a, 0xc4, 0xa8, 0xaa, 0xaa, 0x92, 0xfc, 0x02,
        0x3d, 0x00, 0xc2, 0x07, 0x45, 0xe9, 0x80, 0x00,
        0x00, 0x00
    };

    const char expected_out[129] =
        "house mouse cause 12345 drummer 345 mouse once "
        "the house is 1234 or 12345 drummer 345 mouse tin "
        "drum in the house of once zztop.";

    // The compression is (128 + 7) / 98 so to about 70% of
    // the original size, just because for this short file
    // the gzip format adds a 17% overhead.

    plx::GZip gzip;
    auto rv = gzip.extract(plx::RangeFromArray(gzip_data));
    CheckEQ(rv, true);
    CheckEQ(gzip.output().size(), 128);
    CheckEQ(memcmp(expected_out, gzip.output().start(), 128), 0);
    CheckEQ(gzip.file_name(), "001.txt");
  }

  {
    // gzip stream with a single deflate (08) block.
    // deflate uses the fixed (1) dictionary but unlike the
    // previous case it uses deltas of (1, 65) when the output
    // is just one byte, because the input is long sequences of
    // '0' and '1'.
    const uint8_t gzip_data[39] = {
        0x1f, 0x8b, 0x08, 0x08, 0x44, 0xf5, 0x15, 0x54,
        0x00, 0x0b, 0x30, 0x30, 0x32, 0x2e, 0x74, 0x78,
        0x74, 0x00, 0x33, 0x30, 0xa0, 0x0c, 0x18, 0x52,
        0x08, 0x28, 0xb4, 0x9e, 0x62, 0xfb, 0x01, 0x45,
        0xb4, 0x53, 0x09, 0x00, 0x01, 0x00, 0x00
    };

    plx::GZip gzip;
    auto rv = gzip.extract(plx::RangeFromArray(gzip_data));
    CheckEQ(rv, true);
    CheckEQ(gzip.output().size(), 256);
    CheckEQ(gzip.file_name(), "002.txt");
  }

  {
    plx::GZip gzip;
    {
      // gzip stream with a single deflate (08) block.
      // deflate uses a dynamic (2) dictionary.
      plx::FilePath fp(L"data\\compression\\003.txt.gz");
      auto param = plx::FileParams::Read_SharedRead();
      plx::File f = plx::File::Create(fp, param, plx::FileSecurity());
      CheckEQ(f.size_in_bytes(), 1041UL);

      plx::Range<uint8_t> gzip_data(nullptr, f.size_in_bytes());
      auto mem_holder = plx::HeapRange(gzip_data);
      f.read(gzip_data, 0);
      // Compression is about 50%.
      auto rv = gzip.extract(gzip_data.const_bytes());
      CheckEQ(rv, true);
      CheckEQ(gzip.output().size(), 2038);
      CheckEQ(gzip.file_name(), "003.txt");
    }
    {
      // Now compare with the plaintext. It must be the same.
      plx::FilePath fp(L"data\\compression\\003.txt");
      auto param = plx::FileParams::Read_SharedRead();
      plx::File f = plx::File::Create(fp, param, plx::FileSecurity());
      CheckEQ(f.size_in_bytes(), 2038UL);
      plx::Range<uint8_t> source_data(nullptr, f.size_in_bytes());
      auto mem_holder = plx::HeapRange(source_data);
      f.read(source_data, 0);
      CheckEQ(memcmp(source_data.start(), gzip.output().start(), 2038), 0);
    }
  }

}

void Test_ReaderWriterLock::Exec() {
  {
    plx::ReaderWriterLock rw_lock;
    {
      auto sl1 = rw_lock.write_lock();
    }
    {
      auto sl2 = rw_lock.read_lock();
    }
  }

  {
    plx::ReaderWriterLock rw_lock;
    int value = 1;
    int max = 0;

    auto lam = [&max](int& var, plx::ReaderWriterLock& rw_lock) {
      while (true) {
        auto reader = rw_lock.read_lock();
        if (var > 20) {
          break;
        }
      }

      auto writer = rw_lock.write_lock();
      max = var;
      var = 0;
    };

    std::thread t1(lam, std::ref(value), std::ref(rw_lock));
    while (true) {
      auto writer = rw_lock.write_lock();
      if (value == 0)
        break;
      ++value;
    }

    t1.join();
    CheckEQ(max > 20, true);
  }

}

namespace plx {
class TestService {
  int count_;
public:
  TestService() : count_(66) {}
  int count() const { return count_; }
  void increment() { ++count_; }
};
}

plx::Globals globals;

void Test_Globals::Exec() {
  globals.add_service(new plx::TestService);

  auto svc = plx::Globals::get<plx::TestService>();
  svc->increment();
  CheckEQ(67, svc->count());
}

bool WriteMemory(uint8_t* ptr, uint8_t val) {
  __try {
    *ptr = val;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
  return true;
}

void Test_VEHManager::Exec() {
  globals.add_service(new plx::VEHManager);
  auto vehm = plx::Globals::get<plx::VEHManager>();

  void* addr;
  plx::VEHManager::HandlerFn hf([&addr](EXCEPTION_RECORD* er) -> bool {
    addr = reinterpret_cast<void*>(er->ExceptionInformation[1]);
    return false;
  });

  plx::Range<uint8_t> range(0, 20);
  range.advance(10);

  addr = nullptr;
  vehm->add_av_handler(range, hf);
  CheckEQ(WriteMemory(range.start(), 7), false);
  CheckEQ(ULONG_PTR(addr), 10);

  addr = nullptr;
  vehm->remove_av_handler(range);
  CheckEQ(WriteMemory(range.start(), 8), false);
  CheckEQ(ULONG_PTR(addr), 0);
}

