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

} // namespace JsonImp

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

}
