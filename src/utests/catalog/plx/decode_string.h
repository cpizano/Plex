//#~def plx::DecodeString
///////////////////////////////////////////////////////////////////////////////
namespace plx {
std::string DecodeString(plx::Range<const char>& range) {
  if (range.empty())
    return std::string();
  if (range[0] != '\"') {
    auto r = plx::RangeFromBytes(range.start(), 1);
    throw plx::CodecException(__LINE__, &r);  //#~ln(plx.ds.mis)
  }

  std::string s;
  for (;;) {
    auto text_start = range.start();
    while (range.advance(1) > 0) {
      auto c = range.front();
      if (c < 32) {
        throw plx::CodecException(__LINE__, nullptr);  //#~ln(plx.ds.zer)
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
    throw plx::CodecException(__LINE__, nullptr);  //#~ln(plx.ds.eor)

  escape:
    s.append(++text_start, range.start());
    if (range.advance(1) <= 0)
      throw plx::CodecException(__LINE__, nullptr);  //#~ln(plx.ds.mes)

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

}
