// test_002, part of the plex test suite.

namespace oxen {
#if 1
class Foo {
 public :
  bool one() const { return true; }
  float two() const { return 22.01; }
};
#endif
#if 0
  this is insane stuff .... here
  here as well.
#endif
// now.
bool func() {
  __if_exists(barf) {
    return false;
  }
  __if_exists(burp) {
    return true;
  }
}
}  // namespace "what if?"

