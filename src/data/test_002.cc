// Some comment here

namespace oxen {
#if 1
class Foo {
 public :
  bool one() const { return true; }
  float two() const { return 22.01; }
};
#endif
}  // namespace

#pragma plex_test token_count 3 3
#pragma plex_test token_count 4 2
#pragma plex_test token_count 5 3
#pragma plex_test token_count 6 2
#pragma plex_test token_count 7 10
#pragma plex_test token_count 8 10
#pragma plex_test token_count 9 2
#pragma plex_test token_count 10 1
#pragma plex_test token_count 11 2
#pragma plex_test name_count 9
