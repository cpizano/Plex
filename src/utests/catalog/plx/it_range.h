//#~def plx::ItRange
///////////////////////////////////////////////////////////////////////////////
// plx::ItRange
// s_ : first element
// e_ : one past the last element
// 
namespace plx {
template <typename It>
class ItRange {             //#~leaf class
  It s_;
  It e_;

public:
  typedef typename std::iterator_traits<
      typename std::remove_reference<It>::type
  >::value_type ValueT; 

  ItRange() : s_(), e_() {
  }

  ItRange(It start, It end) : s_(start), e_(end) {
  }

  ItRange(It start, size_t size) : s_(start), e_(start + size) {
  }

  bool empty() const {
    return (s_ == e_);
  }

  size_t size() const {
    return (e_ - s_);
  }

  It start() const {
    return s_;
  }

  It end() const {
    return e_;
  }

  void clear() {
    s_ = It();
    e_ = It();
  }

  ValueT& front() {                 //#~ assert(s_ < e_)
    return s_[0];
  }

  ValueT& back() {                  //#~ assert(s_ < e_)
    return e_[-1];
  }

  const ValueT& front() const {    //#~ assert(s_ < e_)
    return s_[0];
  }

  const ValueT& back() const {     //#~ assert(s_ < e_)
    return e_[-1];
  }

  ValueT& operator[](size_t i) {    //#~ assert(s_ < e_)
    return s_[i];
  }

  const ValueT& operator[](size_t ix) const {
    return s_[ix];
  }

};

}
