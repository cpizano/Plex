//#~def plx::UTF16FromUTF8
///////////////////////////////////////////////////////////////////////////////
// plx::UTF16FromUTF8

namespace plx {
std::wstring UTF16FromUTF8(const plx::Range<const uint8_t>& utf8) {
  if (utf8.empty())
      return std::wstring();
  // Get length and validate string.
  const int utf16_len = ::MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS,
      reinterpret_cast<const char*>(utf8.start()),
      plx::To<int>(utf8.size()),
      NULL,
      0);
  if (utf16_len == 0) {
    throw plx::CodecException(__LINE__, nullptr);
  }

  std::wstring utf16;
  utf16.resize(utf16_len);
  // Now do the conversion without validation.
  if (!::MultiByteToWideChar(
      CP_UTF8, 0,
      reinterpret_cast<const char*>(utf8.start()),
      plx::To<int>(utf8.size()),
      &utf16[0],
      utf16_len)) {
    throw plx::CodecException(__LINE__, nullptr);
  }
  return utf16;
}
}
