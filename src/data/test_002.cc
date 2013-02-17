// Some comment here

namespace oxen {
#if 1
class Foo {
 public :
  bool one() const { return true; }
  float two() const { return 22.01; }
};
#else
class Foo {
 public :
  bool one() const { return false; }
  float two() const { return 33.002; }
};
#endif
}  // namespace

#pragma plex_test token_count 3 5
#pragma plex_test token_count 4 5
#pragma plex_test token_count 5 0
#pragma plex_test token_count 6 6
#pragma plex_test token_count 7 8
#pragma plex_test token_count 8 8
#pragma plex_test token_count 9 5
#pragma plex_test token_count 10 0
#pragma plex_test name_count 6
