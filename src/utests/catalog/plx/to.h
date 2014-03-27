//#~def plx::To
///////////////////////////////////////////////////////////////////////////////
// plx::To  (integer to integer type safe cast)
namespace plx {

template <bool src_signed, bool tgt_signed>
struct ToCastHelper;

template <>
struct ToCastHelper<false, false> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (sizeof(Tgt) >= sizeof(Src)) {
      return static_cast<Tgt>(value);
    } else {
      if (value > std::numeric_limits<Tgt>::max())
        throw plx::OverflowException(__LINE__, OverflowKind::Positive);
      if (value < std::numeric_limits<Tgt>::min())
        throw plx::OverflowException(__LINE__, OverflowKind::Negative);
      return static_cast<Tgt>(value);
    }
  }
};

template <>
struct ToCastHelper<true, true> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (sizeof(Tgt) >= sizeof(Src)) {
      return static_cast<Tgt>(value);
    } else {
      if (value > std::numeric_limits<Tgt>::max())
        throw plx::OverflowException(__LINE__, OverflowKind::Positive);
      if (value < std::numeric_limits<Tgt>::min())
        throw plx::OverflowException(__LINE__, OverflowKind::Negative);
      return static_cast<Tgt>(value);
    }
  }
};

template <>
struct ToCastHelper<false, true> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (value > std::numeric_limits<Tgt>::max())
      throw plx::OverflowException(__LINE__, OverflowKind::Positive);
    if (value < std::numeric_limits<Tgt>::min())
      throw plx::OverflowException(__LINE__, OverflowKind::Negative);
    return static_cast<Tgt>(value);
  }
};

template <>
struct ToCastHelper<true, false> {
  template <typename Tgt, typename Src>
  static inline Tgt cast(Src value) {
    if (value < Src(0))
      throw plx::OverflowException(__LINE__, OverflowKind::Negative);
    if (unsigned(value) > std::numeric_limits<Tgt>::max())
      throw plx::OverflowException(__LINE__, OverflowKind::Positive);
    return static_cast<Tgt>(value);
  }
};

template <typename Tgt, typename Src>
typename std::enable_if<
    std::numeric_limits<Tgt>::is_integer &&
    std::numeric_limits<Src>::is_integer,
    Tgt>::type
To(const Src & value) {
  return ToCastHelper<std::numeric_limits<Src>::is_signed,
                      std::numeric_limits<Tgt>::is_signed>::cast<Tgt>(value);
}
}
