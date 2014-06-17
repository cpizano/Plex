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
    CheckEQ((count_files > 3800) && (count_files < 4000), true);
    CheckEQ((count_dirs > 100) && (count_dirs < 120), true);
  }

  {
    auto fp = plx::GetExePath();
    auto par = plx::FileParams::Directory_ShareAll();
    plx::File f = plx::File::Create(fp, par, plx::FileSecurity());
    CheckEQ(f.status(), plx::File::directory | plx::File::existing);
  }

}
