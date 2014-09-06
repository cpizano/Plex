//#~def plx::Hash_FNV1a
///////////////////////////////////////////////////////////////////////////////
// plx::Hash_FNV1a  (nice hash function for strings used by c++ std)
//
namespace plx {
// Test "foobar" --> 0x85944171f73967e8ULL.
size_t Hash_FNV1a(const plx::Range<const char>& r) {
  auto bp = reinterpret_cast<const unsigned char*>(r.start());
  auto be = reinterpret_cast<const unsigned char*>(r.end());

  uint64_t hval = 0xcbf29ce484222325ULL;
  while (bp < be) {
    // xor the bottom with the current octet.
    hval ^= (uint64_t)*bp++;
    // multiply by the 64 bit FNV magic prime mod 2^64. In other words
    // hval *= FNV_64_PRIME; which is 0x100000001b3ULL;
    hval += (hval << 1) + (hval << 4) + (hval << 5) +
            (hval << 7) + (hval << 8) + (hval << 40);
  }
  // Microsoft's std::string hash has: hval ^= hval >> 32;
  return hval;
}

}
