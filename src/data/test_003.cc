// test_003, part of the plex test suite.

typedef float V;
long global_dec;

V bar(V x, V y, int z) {
  if (--z >= 3)
    return x + y * (z - 1);
  else
    return y + z;  // TODO(fixme).
}

bool baz(unsigned int pp, char* arr) {
  const float lema = (pp == 3)? 3.0f : 12.11123f;
  global_dec += arr[1];
  return float(arr[0]++) != lema;
}

