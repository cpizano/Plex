// test_003, part of the plex test suite.

typedef float V;
long global_dec;

V bar(V x, V y, int z) {
  if (--z >= 3)
    return x + y * (z - 1);
  else
    return y + z;
}

#if defined(plex)
#pragma plex_test name_count 15
#endif

bool baz(unsigned int pp, char* arr) {
  const float lema = (pp == 3)? 3.0f : 12.11123f;
  global_dec += arr[1];
  return float(arr[0]++) != lema;
}

#if defined(plex)
#pragma plex_test token_count 3 4
#pragma plex_test token_count 4 3
#pragma plex_test token_count 5 0

#pragma plex_test token_count 6 13
#pragma plex_test token_count 7 7
#pragma plex_test token_count 8 10   // note: z - 1 is two tokens. 
#pragma plex_test token_count 9 1
#pragma plex_test token_count 10 5
#pragma plex_test token_count 11 1
#pragma plex_test token_count 12 0

#pragma plex_test token_count 17 12
#pragma plex_test token_count 18 14
#pragma plex_test token_count 19 7
#pragma plex_test token_count 20 12
#pragma plex_test token_count 21 1

#pragma plex_test name_count 24

#endif
