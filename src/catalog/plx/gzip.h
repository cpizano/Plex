//#~def plx::GZip
///////////////////////////////////////////////////////////////////////////////
// plx::GZip  : implements a GZip compatible decompressor.
//
namespace plx {

class GZip {
  plx::Inflater inflater_;
  std::string fname_;

public:
  GZip() {
  }

  plx::Range<const uint8_t> output() const {
    return inflater_.output();
  }

  const std::string& file_name() const {
    return fname_;
  }

  bool extract(plx::Range<const uint8_t>& input) {
    plx::BitSlicer slicer(input);
    auto magic = slicer.slice(16);
    if (magic != 0x8b1f)
      return false;
    auto method = slicer.slice(8);
    if (method != 8)
      return false;

    // Flags:
    // bit 0  FTEXT : text hint. Don't care.
    // bit 1  FHCRC : not checked.
    // bit 2  FEXTRA : skipped.
    // bit 3  FNAME : skipped ($$ todo fix).
    // bit 4  FCOMMENT : skipped.
    slicer.slice(1);
    bool has_crc = slicer.slice(1) == 1;
    bool has_extra = slicer.slice(1) == 1;
    bool has_name = slicer.slice(1) == 1;
    bool has_comment = slicer.slice(1) == 1;
    auto reserved = slicer.slice(3);

    if (reserved)
      return false;

    auto mtime = slicer.next_uint32();
    auto xfl = slicer.slice(8);
    auto os = slicer.slice(8);

    if (has_extra) {
      auto xlen = slicer.slice(16);
      slicer.skip(xlen);
    }

    slicer.discard_bits();

    if (has_name) {
      auto name = slicer.remaining_range();
      size_t pos = 0;
      if (!name.contains(0, &pos))
        return false;
      fname_ = reinterpret_cast<const char*>(name.start());
      slicer.skip(pos + 1);
    }

    if (has_comment) {
      auto comment = slicer.remaining_range();
      size_t pos = 0;
      if (!comment.contains(0, &pos))
        return false;
      slicer.skip(pos + 1);
    }

    if (has_crc) {
      auto crc = slicer.slice(16);
    }

    slicer.discard_bits();

    auto consumed = inflater_.inflate(slicer.remaining_range());
    if (!consumed)
      return false;

    slicer.skip(consumed);
    auto crc = slicer.next_uint32();
    // $$$ todo: chec crc.
    auto isize = slicer.next_uint32();

    if (isize != inflater_.output().size())
      return false;

    input.advance(consumed);
    return true;
  }

};

}
