//#~def plx::NextInt
///////////////////////////////////////////////////////////////////////////////
// plx::NextInt  integer promotion.
namespace plx {

short NextInt(char value) {
  return short(value);
}

int NextInt(short value) {
  return int(value);
}

long long NextInt(int value) {
  return long long(value);
}

long long NextInt(long value) {
  return long long(value);
}

long long NextInt(long long value) {
  return value;
}

short NextInt(unsigned char value) {
  return short(value);
}

int NextInt(unsigned short value) {
  return int(value);
}

long long NextInt(unsigned int value) {
  return long long(value);
}

long long NextInt(unsigned long value) {
  return long long(value);
}

long long NextInt(unsigned long long value) {
  if (static_cast<long long>(value) < 0LL)
    throw plx::OverflowException(__LINE__, plx::OverflowKind::Positive);
  return long long(value);
}

}
