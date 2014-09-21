//#~def plx::HuffmanCodec
///////////////////////////////////////////////////////////////////////////////
// plx::HuffmanCodec : implements a huffman decoder.
// it maps bit patterns (codes) of up to 15 bits to 16 bit symbols.
//
// |lengths| is the number of bits each symbol should use. With 0 bits meaning
// that the symbol does not get a code. Based on that a huffman decoder zlib
// compatible can be constructed like RFC1951 documents.
// 
namespace plx {

class HuffmanCodec {
  std::vector<uint16_t> symbols_;
  std::vector<uint16_t> counts_;

public:
  HuffmanCodec(size_t max_bits, const std::vector<uint16_t>& lengths) {
    if (max_bits > 15)
      throw plx::InvalidParamException(__LINE__, 1);

    counts_.resize(max_bits + 1);
    for (auto len : lengths) {
      counts_[len]++;
    }

    // all codes taking 0 bits makes no sense.
    if (counts_[0] == lengths.size())
      throw plx::InvalidParamException(__LINE__, 2);

    // compute how many symbols are not coded. |left| < 0 means the
    // code is over-subscribed (error), |left| == 0 means every
    // code maps to a symbol.
    int left = 1;
    for (size_t bit_len = 1; bit_len <= max_bits; ++bit_len) {
      left *= 2;
      left -= counts_[bit_len];
      if (left < 0)
        throw plx::InvalidParamException(__LINE__, 2);
    }

    // Generate offsets for each bit lenght.
    std::vector<uint16_t> offsets;
    offsets.resize(max_bits + 1);
    offsets[0] = 0;
    for (size_t len = 1; len != max_bits; ++len) {
      offsets[len + 1] = offsets[len] + counts_[len];
    }

    symbols_.resize(lengths.size());
    uint16_t symbol = 0;
    for (auto losym : lengths) {
      if (losym)
        symbols_[offsets[losym]++] = symbol;
      ++symbol;
    }
  }

  int decode(plx::BitSlicer& slicer) {
    const size_t max_bits = counts_.size() - 1;
    uint16_t code = 0;
    uint16_t first = 0;
    size_t index = 0;
    // $$ todo, replace for an efficient table lookup.
    for (size_t len = 1; len <= max_bits; ++len) {
      code |= slicer.slice(1);
      auto count = counts_[len];
      auto d = code - first;
      if (count > d )
        return symbols_[index + d];
      index += count;
      first += count;
      first *= 2;
      code <<= 1;
    }
    return -1;
  }

};

}
