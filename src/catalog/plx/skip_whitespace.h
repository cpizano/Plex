//#~def plx::SkipWhitespace
///////////////////////////////////////////////////////////////////////////////
// SkipWhitespace (advances a range as long isspace() is false.
//
namespace plx {
template <typename T>
typename std::enable_if<
    sizeof(T) == 1,
    plx::Range<T>>::type
SkipWhitespace(const plx::Range<T>& r) {
  auto wr = r;
  while (!wr.empty()) {
    if (!std::isspace(wr.front()))
      break;
    wr.advance(1);
  }
  return wr;
}
}
