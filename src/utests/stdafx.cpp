// This is the plex precompiled header cc.
// Do not edit this file by hand.

#include "stdafx.h"



namespace plx {
uint32_t CRC32C(uint32_t crc, const uint8_t *buf, size_t len) {
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
ItRange<uint8_t*> RangeFromBytes(void* start, size_t count) {
  auto s = reinterpret_cast<uint8_t*>(start);
  return ItRange<uint8_t*>(s, s + count);
}
ItRange<const uint8_t*> RangeFromBytes(const void* start, size_t count) {
  auto s = reinterpret_cast<const uint8_t*>(start);
  return ItRange<const uint8_t*>(s, s + count);
}
char* HexASCII(uint8_t byte, char* out) {
  *out++ = HexASCIITable[(byte >> 4) & 0x0F];
  *out++ = HexASCIITable[byte & 0x0F];
  return out;
}
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
plx::FilePath GetAppDataPath(bool roaming) {
  auto folder = roaming? FOLDERID_RoamingAppData : FOLDERID_LocalAppData;
  wchar_t* path = nullptr;
  auto hr = ::SHGetKnownFolderPath(folder, 0, nullptr, &path);
  if (hr != S_OK)
    throw plx::IOException(__LINE__, L"<appdata folder>");
  auto fp = FilePath(path);
  ::CoTaskMemFree(path);
  return fp;
}
plx::FilePath GetExePath() {
  wchar_t* pp = nullptr;
  _get_wpgmptr(&pp);
  return FilePath(pp).parent();
}
uint32_t Hash_FNV1a_32(const plx::Range<const uint8_t>& r) {
  auto bp = r.start();
  auto be = r.end();

  uint32_t hval = 0x811c9dc5UL;
  while (bp < be) {
    // xor the bottom with the current octet.
    hval ^= (uint32_t)*bp++;
    // multiply by the 32 bit FNV magic prime mod 2^32. In other words
    // hval *= FNV_32_PRIME; which is 0x01000193UL;
    hval += (hval << 1) + (hval << 4) + (hval << 7) +
            (hval << 8) + (hval << 24);
  }
  return hval;
}
uint64_t Hash_FNV1a_64(const plx::Range<const uint8_t>& r) {
  auto bp = r.start();
  auto be = r.end();

  uint64_t hval = 0xcbf29ce484222325ULL;
  while (bp < be) {
    // xor the bottom with the current octet.
    hval ^= (uint64_t)*bp++;
    // multiply by the 64 bit FNV magic prime mod 2^64. In other words
    // hval *= FNV_64_PRIME; which is 0x100000001b3ULL;
    hval += (hval << 1) + (hval << 4) + (hval << 5) +
            (hval << 7) + (hval << 8) + (hval << 40);
  }
  return hval;
}
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
uint64_t LocalUniqueId() {
  LUID luid = {0};
  ::AllocateLocallyUniqueId(&luid);
  ULARGE_INTEGER li = {luid.LowPart, luid.HighPart};
  return li.QuadPart;
}
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
bool PlatformCheck() {
  __if_exists(plex_sse42_support) {
    if (!plx::CpuId().sse42())
      return false;
  }
  return true;
}
char32_t DecodeUTF8(plx::Range<const uint8_t>& ir) {
  if (!ir.valid() || (ir.size() == 0))
    throw plx::CodecException(__LINE__, nullptr);

  uint8_t fst = ir[0];
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
    uint8_t tmp = ir[i];

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

}
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
namespace impl_v {
int vsnprintf(char* buffer, size_t size,
              const char* format, va_list arguments) {
  int length = _vsprintf_p(buffer, size, format, arguments);
  if (length < 0) {
    if (size > 0)
      buffer[0] = 0;
    return _vscprintf_p(format, arguments);
  }
  return length;
}
}
std::string StringPrintf(const char* fmt, ...) {
  int fmt_size = 128;
  std::unique_ptr<char> mem;

  va_list args;
  va_start(args, fmt);

  while (true) {
    mem.reset(new char[fmt_size]);
    int sz = impl_v::vsnprintf(mem.get(), fmt_size, fmt, args);
    if (sz < fmt_size)
      break;
    fmt_size = sz + 1;
  }

  va_end(args);
  return std::string(mem.get());
}
}
