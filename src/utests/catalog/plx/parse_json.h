//#~def plx::ParseJson
///////////////////////////////////////////////////////////////////////////////
namespace plx {
plx::JsonValue ParseJsonValue(plx::Range<const char>& range);

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
      if (!range.advance(1))
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

} // namespace imp

plx::JsonValue ParseJsonValue(plx::Range<const char>& range) {
  range = plx::SkipWhitespace(range);
  if (range.empty())
    throw plx::CodecException(__LINE__, NULL);

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

  auto r = plx::RangeFromBytes(range.start(), range.size());
  throw plx::CodecException(__LINE__, &r);
}

}
