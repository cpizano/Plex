//#~def plx::To
///////////////////////////////////////////////////////////////////////////////
// plx::To  (integer to integer type safe cast)
namespace plx {
template <typename Tgt, typename Src>
Tgt To(const Src & value) {
  if (std::numeric_limits<Tgt>::max() < std::numeric_limits<Src>::max()) {
    if (plx::GreaterThan(value, std::numeric_limits<Tgt>::max()))
      throw plx::OverflowException(__LINE__, OverflowKind::Positive);
  }

  if (std::numeric_limits<Src>::is_signed) {
    if (!std::numeric_limits<Tgt>::is_signed || (sizeof(Src) > sizeof(Tgt))) {
      if (plx::LessThan(value, std::numeric_limits<Tgt>::min()))
        throw plx::OverflowException(__LINE__, OverflowKind::Negative);
    }
  }

  return static_cast<Tgt>(value);
}
}
