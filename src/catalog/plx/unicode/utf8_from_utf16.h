//#~def plx::UTF8FromUTF16
///////////////////////////////////////////////////////////////////////////////
// plx::UTF8FromUTF16

namespace plx {
std::string UTF8FromUTF16(const plx::Range<const uint16_t>& utf16) {
  if (utf16.empty())
      return std::string();
  // compute length.
  const int utf8_len = ::WideCharToMultiByte(
      CP_UTF8, 0,
      reinterpret_cast<const wchar_t*>(utf16.start()),
      plx::To<int>(utf16.size()),
      NULL,
      0,
      NULL, NULL);
  if (utf8_len == 0) {
    throw plx::CodecException(__LINE__, nullptr);
  }

  std::string utf8;
  utf8.resize(utf8_len);
  // now do the conversion.
  if (!::WideCharToMultiByte(
      CP_UTF8, 0,
      reinterpret_cast<const wchar_t*>(utf16.start()),
      plx::To<int>(utf16.size()),
      &utf8[0],
      utf8_len,
      NULL, NULL)) {
    throw plx::CodecException(__LINE__, nullptr);
  }
  return utf8;
}
}
