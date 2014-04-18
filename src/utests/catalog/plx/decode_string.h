//#~def plx::DecodeString
///////////////////////////////////////////////////////////////////////////////
namespace plx {
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
    // Advance until an escape or until the end of string.
    while (range.advance(1)) {
      switch (range.front()) {
        case '\"' : break;
        case '\\' : goto escape;
        default: continue;
      }
      s.append(++text_start, range.start());
      return s;
    }

  escape:
    s.append(++text_start, range.start());
    range.advance(1);
    switch (range.front()) {
      case '\"':  s.push_back('\"'); break;
      case '\\':  s.push_back('\\'); break;
      case '/':   s.push_back('/');  break;
      case 'b':   s.push_back('\b'); break;
      case 'f':   s.push_back('\f'); break;
      case 'n':   s.push_back('\n'); break;
      case 'r':   s.push_back('\r'); break;
      case 't':   s.push_back('\t'); break;
      default: {
        auto r = plx::RangeFromBytes(range.start() - 1, 2);
        throw plx::CodecException(__LINE__, &r);
      }
    }
  }
}

}
