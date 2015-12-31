//#~def plx::ParseUnsignedInteger
///////////////////////////////////////////////////////////////////////////////
// plx::ParseUnsignedInteger
//
namespace plx {

template <int base = 10>
uint64_t ParseUnsignedInt(plx::Range<const uint8_t>& r) {
  size_t pos = 0;
  auto v = std::stoull(plx::StringFromRange(r), &pos, base);
  r.advance(pos);
  return v;
}

}
