//#~def plx::BitSlicer
///////////////////////////////////////////////////////////////////////////////
// plx::BitSlicer allows you to slice sequential bits of a byte range.
// bits_ : the last extracted bits not given back to caller
// bit_count_ : how many valid bits in |bits_|.
// pos_ : the current byte position in |r_|.
//
namespace plx {

class BitSlicer {
  const plx::Range<const unsigned char>& r_;
  unsigned long bits_;
  size_t bit_count_;
  size_t pos_;

  unsigned long range_check(bool* eor) {
    if (!eor)
      throw plx::RangeException(__LINE__, 0);
    *eor = true;
    return 0; 
  }

public:
  BitSlicer(const plx::Range<const unsigned char>& r)
      : r_(r), bits_(0), bit_count_(0), pos_(0) {
  }

  unsigned long slice(int needed, bool* eor = nullptr) {
    if (needed > 23) {
      throw plx::RangeException(__LINE__, 0);
    }

    unsigned long value = bits_;

    while (bit_count_ < needed) {
      if (past_end())
        return range_check(eor);
      // accumulate 8 bits at a time.
      value |= static_cast<unsigned long>(r_[pos_++]) << bit_count_;
      bit_count_ += 8;
    }

    // store the uneeded bits from above. We will use them
    // for the next request.
    bits_ = value >> needed;
    bit_count_ -= needed;

    // mask out the bits above |needed|.
    return value & ((1L << needed) - 1);
  }

  bool past_end() const {
    return (pos_ == r_.size());
  }

};

}
