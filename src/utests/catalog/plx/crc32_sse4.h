//#~def plx::CRC32C
///////////////////////////////////////////////////////////////////////////////
// plx::CRC32C (computes CRC-32 checksum, SSE4 accelerated)
// Polinomal 0x1EDC6F41 aka iSCSI CRC. This gives a 10^-41 probabilty of not
// detecting a 3-bit burst error and 10^-40 for sporadic one bit errors.
//
namespace plx {
uint32_t CRC32C(uint32_t crc, const char *buf, size_t len) {
  if (len == 0)
    return crc;
  crc ^= 0xFFFFFFFF;
  // Process one byte at a time until aligned.
  for (; (len > 0) && (reinterpret_cast<uintptr_t>(buf) & 0x07); len--, buf++) {
    crc = _mm_crc32_u8(crc, *buf);
  }
  // Then operate 4 bytes at a time.
  for (; len >= sizeof(uint32_t); len -= sizeof(uint32_t), buf += sizeof(uint32_t)) {
    crc = _mm_crc32_u32(crc, *(uint32_t *) (buf));
  }
  // Then process at most 3 more bytes.
  for (; len >= sizeof(uint8_t); len -= sizeof(uint8_t), buf += sizeof(uint8_t)) {
    crc = _mm_crc32_u8(crc, *(uint32_t *) (buf));
  }
  return (crc ^= 0xFFFFFFFF);
}
}
