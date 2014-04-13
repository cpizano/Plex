//#~def plx::HexASCIITable
//#~def plx::HexASCII
///////////////////////////////////////////////////////////////////////////////
namespace plx {
static const char HexASCIITable[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

char* HexASCII(uint8_t byte, char* out) {
  *out++ = HexASCIITable[(byte >> 4) & 0x0F];
  *out++ = HexASCIITable[byte & 0x0F];
  return out;
}
}
