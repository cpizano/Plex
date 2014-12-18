//#~def plx::ItRange
//#~def plx::RangeFromBytes
//#~def plx::RangeFromString
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
  >::reference RefT;

  typedef typename std::iterator_traits<
      typename std::remove_reference<It>::type
  >::value_type ValueT;

  typedef typename std::remove_const<It>::type NoConstIt;

  ItRange() : s_(), e_() {
  }

  template <typename U>
  ItRange(const ItRange<U>& other) : s_(other.start()), e_(other.end()) {
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

  It begin() const {
    return s_;
  }

  It end() const {
    return e_;
  }

  bool valid() const {
    return (e_ >= s_);
  }

  RefT front() const {
    if (s_ >= e_)
      throw plx::RangeException(__LINE__, nullptr);
    return s_[0];
  }

  RefT back() const {                  //#~ assert(s_ < e_)
    return e_[-1];
  }

  RefT operator[](size_t i) const {    //#~ assert(s_ < e_)
    return s_[i];
  }

  bool equals(const ItRange<It>& o) const {
    if (o.size() != size())
      return false;
    return (memcmp(s_, o.s_, size()) == 0);
  }

  size_t starts_with(const ItRange<It>& o) const {
    if (o.size() > size())
      return 0;
    return (memcmp(s_, o.s_, o.size()) == 0) ? o.size() : 0; 
  }

  bool contains(const uint8_t* ptr) const {
    return ((ptr >= reinterpret_cast<uint8_t*>(s_)) &&
            (ptr < reinterpret_cast<uint8_t*>(e_)));
  }

  bool contains(ValueT x, size_t* pos) const {
    auto c = s_;
    while (c != e_) {
      if (*c == x) {
        *pos = c - s_;
        return true;
      }
      ++c;
    }
    return false;
  }

  template <size_t count>
  size_t CopyToArray(ValueT (&arr)[count]) const {
    auto copied = std::min(size(), count);
    auto last = copied + s_;
    std::copy(s_, last, arr);
    return copied;
  }

  template <size_t count>
  size_t CopyToArray(std::array<ValueT, count>& arr) const {
    auto copied = std::min(size(), count);
    auto last = copied + s_;
    std::copy(s_, last, arr.begin());
    return copied;
  }

  intptr_t advance(size_t count) {
    auto ns = s_ + count;
    if (ns > e_)
      return (e_ - ns);
    s_ = ns;
    return size();
  }

  void clear() {
    s_ = It();
    e_ = It();
  }

  void reset_start(It new_start) {
    auto sz = size();
    s_ = new_start;
    e_ = s_ + sz;
  }

  void extend(size_t count) {
    e_ += count;
  }

  ItRange<const uint8_t*> const_bytes() const {
    auto s = reinterpret_cast<const uint8_t*>(s_);
    auto e = reinterpret_cast<const uint8_t*>(e_);
    return ItRange<const uint8_t*>(s, e);
  }

  ItRange<uint8_t*> bytes() const {
    auto s = reinterpret_cast<uint8_t*>(s_);
    auto e = reinterpret_cast<uint8_t*>(e_);
    return ItRange<uint8_t*>(s, e);
  }

  ItRange<It> slice(size_t start, size_t count = 0) const {
    return ItRange<It>(s_ + start,
                       count ? (s_ + start + count) : e_ );
  }

};

template <typename U, size_t count>
ItRange<U*> RangeFromLitStr(U (&str)[count]) {
  return ItRange<U*>(str, str + count - 1);
}

template <typename U, size_t count>
ItRange<U*> RangeFromArray(U (&str)[count]) {
  return ItRange<U*>(str, str + count);
}

template <typename U>
ItRange<U*> RangeUntilValue(U* start, U value) {
  auto stop = start;
  while (*stop != value) {
    ++stop;
  }
  return ItRange<U*>(start, stop);
}

template <typename U>
ItRange<U*> RangeFromVector(std::vector<U>& vec, size_t len = 0) {
  auto s = &vec[0];
  return ItRange<U*>(s, len ? s + len : s + vec.size());
}

template <typename U>
ItRange<const U*> RangeFromVector(const std::vector<U>& vec, size_t len = 0) {
  auto s = &vec[0];
  return ItRange<const U*>(s, len ? s + len : s + vec.size());
}

ItRange<uint8_t*> RangeFromBytes(void* start, size_t count) {
  auto s = reinterpret_cast<uint8_t*>(start);
  return ItRange<uint8_t*>(s, s + count);
}

ItRange<const uint8_t*> RangeFromBytes(const void* start, size_t count) {
  auto s = reinterpret_cast<const uint8_t*>(start);
  return ItRange<const uint8_t*>(s, s + count);
}

ItRange<const uint8_t*> RangeFromString(const std::string& str) {
  auto s = reinterpret_cast<const uint8_t*>(&str.front());
  return ItRange<const uint8_t*>(s, s + str.size());
}

ItRange<uint8_t*> RangeFromString(std::string& str) {
  auto s = reinterpret_cast<uint8_t*>(&str.front());
  return ItRange<uint8_t*>(s, s + str.size());
}

ItRange<const uint16_t*> RangeFromString(const std::wstring& str) {
  auto s = reinterpret_cast<const uint16_t*>(&str.front());
  return ItRange<const uint16_t*>(s, s + str.size());
}

ItRange<uint16_t*> RangeFromString(std::wstring& str) {
  auto s = reinterpret_cast<uint16_t*>(&str.front());
  return ItRange<uint16_t*>(s, s + str.size());
}

template <typename U>
std::string StringFromRange(const ItRange<U>& r) {
  return std::string(r.start(), r.end());
}

template <typename U>
std::wstring WideStringFromRange(const ItRange<U>& r) {
  return std::wstring(r.start(), r.end());
}

template <typename U>
std::unique_ptr<U[]> HeapRange(ItRange<U*>&r) {
  std::unique_ptr<U[]> ptr(new U[r.size()]);
  r.reset_start(ptr.get());
  return ptr;
}

}
