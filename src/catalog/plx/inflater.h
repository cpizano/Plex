//#~def plx::Inflater
///////////////////////////////////////////////////////////////////////////////
// plx::Inflater  : Implements a decompressor for the DEFLATE format following
//                  RFC 1951. It takes a compressed byte range |r| and returns
//                  via output() the decompressed data.
namespace plx {
class Inflater {
  int error_;
  std::vector<uint8_t> output_;
  std::unique_ptr<plx::HuffmanCodec> liter_len_;
  std::unique_ptr<plx::HuffmanCodec> distance_;

public:
  enum Errors {
    success = 0,
    not_implemented,
    invalid_block_type,
    invalid_length,
    empty_block,
    missing_data,
    invalid_symbol,
    bad_dynamic_dict,
    too_many_lengths,
    bad_huffman
  };

  Inflater() : error_(success)  {
  }

  const std::vector<uint8_t>& output() const {
    return output_;
  }

  Errors status() const { return Errors(error_); }

  size_t inflate(const plx::Range<const uint8_t>& r) {
    if (r.empty()) {
      error_ = empty_block;
      return 0;
    }

    error_ = success;
    plx::BitSlicer slicer(r);

    while (true) {
      auto last = slicer.slice(1) == 1;
      auto type = slicer.slice(2);

      switch (type) {
      case 0:  error_ = stored(slicer);
        break;
      case 1:  error_ = fixed(slicer);
        break;
      case 2:  error_ = dynamic(slicer);
        break;
      default:
        error_ = invalid_block_type;
        break;
      }

      if (error_) {
        break;
      } else if (last) {
        break;
      } else if (slicer.past_end()) {
        error_ = missing_data;
        break;
      }
    }

    return (error_ == success) ? slicer.pos() : 0;
  }

  Errors stored(plx::BitSlicer& slicer) {
    // The size of the block comes next in two bytes and its complement.
    slicer.discard_bits();
    unsigned int len = slicer.slice(16);
    unsigned int clen = slicer.slice(16);
    // check complement.
    if (len != ((~clen) & 0x0ffff))
      return invalid_length;

    if (!len)
      return empty_block;

    // copy |len| bytes to the output.
    const auto& remaining = slicer.remaining_range();
    if (remaining.size() < len)
      return missing_data;

    output_.insert(output_.end(), remaining.start(), remaining.start() + len);
    slicer.skip(len);
    return success;
  }

  Errors fixed(plx::BitSlicer& slicer) {
    construct_fixed_codecs();
    return decode(slicer, *liter_len_, *distance_);
  }

  Errors dynamic(plx::BitSlicer& slicer) {
    // number of Literal & Length codes, from 257 to 286.
    auto ll_len = slicer.slice(5) + 257;
    // number of distance codes, from 1 to 32.
    auto dist_len = slicer.slice(5) + 1;
    // number of code length codes, from 4 to 19.
    auto clc_len = slicer.slice(4) + 4;

    if (ll_len > 286)
      return bad_dynamic_dict;
    if (dist_len > 30)
      return bad_dynamic_dict;

    // the |order| represents the order most common symbols expected from the
    // next decoding phase: 16 = rle case 1, 17 = rle case 2 and 18 = case 3,
    // then 0 = no code assigned. The rest are bit lenghts.
    static const size_t order[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    // Build the huffman decoder for the code length codes.
    std::vector<uint16_t> lengths;
    lengths.resize(19);
    size_t index = 0;
    for (; index != clc_len; ++index)
      lengths[order[index]] = static_cast<uint16_t>(slicer.slice(3));
    for (; index < 19; ++index)
      lengths[order[index]] = 0;
    plx::HuffmanCodec clc_huffman(15, plx::RangeFromVector(lengths));

    // Now read the real huffman decoder for the dynamic block.
    const auto table_len = ll_len + dist_len;

    lengths.resize(table_len);
    size_t ix = 0;

    while (ix < table_len) {
      auto symbol = clc_huffman.decode(slicer);
      if (symbol < 0)
        return invalid_symbol;
      if (symbol < 16) {
        // the symbol is the huffman bit lenght.
        lengths[ix++] = static_cast<uint16_t>(symbol);
      } else {
        // run length encoded. There are 3 cases.
        int rle_value;
        int rle_count;

        if (symbol == 16) {
          if (!ix)
            return bad_dynamic_dict;
          // case 1: repeat last bit length value 3 to 6 times.
          rle_value = lengths[ix - 1];
          rle_count = 3 + slicer.slice(2);
        } else {
          rle_value = 0;
          if (symbol == 17) {
            // case 2: 0 bit length (no code for symbol) is repeated 3 to 10 times.
            rle_count = 3 + slicer.slice(3);
          } else {
            // case 3: 0 bit length (no code for symbol) is repeated 11 to 138 times.
            rle_count = 11 + slicer.slice(7);
          }
        }

        if (ix + rle_count > table_len)
          return too_many_lengths;

        while (rle_count--)
          lengths[ix++] = static_cast<uint16_t>(rle_value);
      }
    }

    // we should have received a non-zero bit length for code 256, because
    // that is how the decompressor will know how to stop.
    if (!lengths[256])
      return bad_huffman;

    // Finally construct the two huffman decoders and decode the stream.
    auto rfv = plx::RangeFromVector(lengths);
    plx::HuffmanCodec lit_len(15, rfv.slice(0, ll_len));
    plx::HuffmanCodec distance(15, rfv.slice(ll_len));

    return decode(slicer, lit_len, distance);
  }

  void construct_fixed_codecs() {
    if (liter_len_ && distance_)
      return;

    const size_t fixed_ll_codes = 288;
    const size_t fixed_dis_codes = 30;

    std::vector<uint16_t> lengths;
    lengths.resize(fixed_ll_codes);
    size_t symbol;

    for (symbol = 0; symbol != 144; ++symbol)
      lengths[symbol] = 8;
    for (; symbol != 256; ++symbol)
      lengths[symbol] = 9;
    for (; symbol != 280; ++symbol)
      lengths[symbol] = 7;
    for (; symbol != fixed_ll_codes; ++symbol)
      lengths[symbol] = 8;

    // Symbol 286 and 287 should never appear in the stream.
    liter_len_.reset(new plx::HuffmanCodec(15, plx::RangeFromVector(lengths)));

    lengths.resize(fixed_dis_codes);
    for (symbol = 0; symbol != fixed_dis_codes; ++symbol)
      lengths[symbol] = 5;

    distance_.reset(new plx::HuffmanCodec(15, plx::RangeFromVector(lengths)));
  }

  Errors decode(plx::BitSlicer& slicer, plx::HuffmanCodec& len_lit, plx::HuffmanCodec& dist) {
    while (true) {
      // either a literal or the length of a <len, dist> pair.
      auto symbol = len_lit.decode(slicer);
      if (symbol < 0)
        return invalid_symbol;

      if (symbol == 256) {
        // special symbol that signifies the end of stream.
        return success;
      } else if (symbol < 256) {
        // literal.
        output_.push_back(static_cast<uint8_t>(symbol));
      } else {
        // above 256 means length - distance pair.
        auto len = decode_length(symbol, slicer);
        if (len < 0)
          return invalid_symbol;
        symbol = dist.decode(slicer);
        if (symbol < 0)
          return invalid_symbol;
        auto dist = decode_distance(symbol, slicer);
        // copy data from the already decoded stream.
        back_copy(dist, len);
      }
    }  // while. 
  }

  //$$$ todo: remove the static arrays below, they are not thread safe.

  int decode_length(int symbol, plx::BitSlicer& slicer) {
    static const int lens[29] = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
    };
    static const int lext[29] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
    };

    symbol -= 257;
    if (symbol >= 29)
      return -1;

    auto extra_bits = lext[symbol];
    return lens[symbol] +  (extra_bits ? slicer.slice(extra_bits) : 0);
  }

  int decode_distance(int symbol, plx::BitSlicer& slicer) {
    static const int dists[30] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577
    };

    static const short dext[30] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
    };

    if (symbol >= 29)
      return -1;

    auto extra_bits = dext[symbol];
    return dists[symbol] + (extra_bits ? slicer.slice(extra_bits) : 0);
  }

  void back_copy(int from, int count) {
    auto s = output_.size() - from;
    for (int ix = 0; ix != count; ++ix)
      output_.insert(output_.end(), output_[s++]);
  }

};

}
