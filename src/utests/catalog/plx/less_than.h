//#~def plx::LessThan
///////////////////////////////////////////////////////////////////////////////
// plx::LessThan
namespace plx {
template <typename LHS>
typename std::enable_if<
    std::numeric_limits<LHS>::is_signed &&
    std::numeric_limits<LHS>::is_integer,
    bool>::type
LessThan(LHS const lhs, long rhs) {
  return (lhs < rhs);
}

template <typename LHS>
typename std::enable_if<
    !std::numeric_limits<LHS>::is_signed &&
    std::numeric_limits<LHS>::is_integer,
    bool>::type
LessThan(LHS const lhs, unsigned long rhs) {
  return (lhs < rhs);
}

template <typename LHS>
typename std::enable_if<
    std::numeric_limits<LHS>::is_signed &&
    std::numeric_limits<LHS>::is_integer &&
    (std::numeric_limits<LHS>::digits > std::numeric_limits<long>::digits),
    bool>::type
LessThan(LHS const lhs, long long rhs) {
  return (lhs < rhs);
}

template <typename LHS>
typename std::enable_if<
    !std::numeric_limits<LHS>::is_signed &&
    std::numeric_limits<LHS>::is_integer &&
    (std::numeric_limits<LHS>::digits > std::numeric_limits<unsigned long>::digits),
    bool>::type
LessThan(LHS const lhs, unsigned long long rhs) {
  return (lhs < rhs);
}

template <typename LHS>
typename std::enable_if<
    !std::numeric_limits<LHS>::is_integer, bool>::type
LessThan(LHS const lhs, double rhs) {
  return (lhs < rhs);
}
}
