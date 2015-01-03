//#~def plx::Hash_FNV1a_32
///////////////////////////////////////////////////////////////////////////////
// plx::Hash_FNV1a_32  (nice hash function for strings used by c++ std)
// for short inputs is about 100 times faster than SHA1 and about 20 times
// faster for long inputs.
namespace plx {
uint32_t Hash_FNV1a_32(const plx::Range<const uint8_t>& r) {
  auto bp = r.start();
  auto be = r.end();

  uint32_t hval = 0x811c9dc5UL;
  while (bp < be) {
    // xor the bottom with the current octet.
    hval ^= (uint32_t)*bp++;
    // multiply by the 32 bit FNV magic prime mod 2^32. In other words
    // hval *= FNV_32_PRIME; which is 0x01000193UL;
    hval += (hval << 1) + (hval << 4) + (hval << 7) +
            (hval << 8) + (hval << 24);
  }
  return hval;
}
}
