//#~def plx::ParseSignedInteger
///////////////////////////////////////////////////////////////////////////////
// plx::ParseSignedInteger
//
namespace plx {

template<int base = 10>
int64_t ParseSignedInt(plx::Range<const uint8_t>& r) {
  size_t pos = 0;
  auto v = std::stoll(plx::StringFromRange(r), &pos, base);
  r.advance(pos);
  return v;
}

}
