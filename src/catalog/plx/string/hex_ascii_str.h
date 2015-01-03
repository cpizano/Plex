//#~def plx::HexASCIIStr
///////////////////////////////////////////////////////////////////////////////
namespace plx {
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
}
