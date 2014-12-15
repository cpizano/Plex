//#~def plx::DecodeUTF8
///////////////////////////////////////////////////////////////////////////////
// plx::DecodeUTF8 (decodes a UTF8 codepoint into a 32-bit codepoint)
//
// bits encoding
// 7    0xxxxxxx
// 11   110xxxxx 10xxxxxx
// 16   1110xxxx 10xxxxxx 10xxxxxx
// 21   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// The 5 and 6 bytes encoding are no longer valid since RFC 3629. 
// max point is then U+10FFFF.
//
namespace plx {
static const uint32_t Utf8BitMask[] = {
  (1 << 7) - 1,   // 0000 0000 0000 0000 0111 1111
  (1 << 11) - 1,  // 0000 0000 0000 0111 1111 1111
  (1 << 16) - 1,  // 0000 0000 1111 1111 1111 1111
  (1 << 21) - 1   // 0001 1111 1111 1111 1111 1111
};

char32_t DecodeUTF8(plx::Range<const uint8_t>& ir) {
  if (!ir.valid() || (ir.size() == 0))
    throw plx::CodecException(__LINE__, nullptr);

  uint8_t fst = ir[0];
  if (!(fst & 0x80)) {
    // One byte sequence, so we are done.
    ir.advance(1);
    return fst;
  }

  if ((fst & 0xC0) != 0xC0)
    throw plx::CodecException(__LINE__, &ir);

  uint32_t d = fst;
  fst <<= 1;

  for (unsigned int i = 1; (i != 3) && (ir.size() > i); ++i) {
    uint8_t tmp = ir[i];

    if ((tmp & 0xC0) != 0x80)
      throw plx::CodecException(__LINE__, &ir);

    d = (d << 6) | (tmp & 0x3F);
    fst <<= 1;

    if (!(fst & 0x80)) {
      d &= Utf8BitMask[i];

      // overlong check.
      if ((d & ~Utf8BitMask[i - 1]) == 0)
        throw plx::CodecException(__LINE__, &ir);

      // surrogate check.
      if (i == 2) {
        if ((d >= 0xD800 && d <= 0xDFFF) || d > 0x10FFFF)
          throw plx::CodecException(__LINE__, &ir);
      }

      ir.advance(i + 1);
      return d;
    }

  }
  throw plx::CodecException(__LINE__, &ir);
}
}
