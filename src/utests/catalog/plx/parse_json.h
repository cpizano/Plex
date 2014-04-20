//#~def plx::ParseJson
///////////////////////////////////////////////////////////////////////////////
namespace plx {
namespace imp {
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
} // namespace imp

plx::JsonValue ParseJsonValue(plx::Range<const char>& range) {
  range = plx::SkipWhitespace(range);
  if (range.empty())
    throw 5;
  if (range.front() == '\"')
    return plx::DecodeString(range);
  if (imp::Consume(range, "true"))
    return true;
  if (imp::Consume(range, "false"))
    return false;
  if (imp::Consume(range, "null"))
    return nullptr;
  throw 5;
}

}
