//#~def plx::Hash_FNV1a_64
///////////////////////////////////////////////////////////////////////////////
// plx::Hash_FNV1a_64  (nice hash function for strings used by c++ std)
// for short inputs is about 100 times faster than SHA1 and about 20 times
// faster for long inputs.
namespace plx {
uint64_t Hash_FNV1a_64(const plx::Range<const unsigned char>& r) {
  auto bp = r.start();
  auto be = r.end();

  uint64_t hval = 0xcbf29ce484222325ULL;
  while (bp < be) {
    // xor the bottom with the current octet.
    hval ^= (uint64_t)*bp++;
    // multiply by the 64 bit FNV magic prime mod 2^64. In other words
    // hval *= FNV_64_PRIME; which is 0x100000001b3ULL;
    hval += (hval << 1) + (hval << 4) + (hval << 5) +
            (hval << 7) + (hval << 8) + (hval << 40);
  }
  return hval;
}
}
