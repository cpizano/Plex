// test_006, part of the plex test suite.

#if defined(pex)
#pragma plex_test xdef class laa::External
#pragma plex_test xdef function Roar
#endif

namespace xx {
namespace yy {

class Reman;

class Fooman {
  int m;
  int n;
  Reman* r;
  laa::External e;

public:
  Fooman(const Reman& rr) 
      : m(0), n(1), r(new Reman(rr)), e(0) {
  }

  Fooman(laa::External& x) 
      : m(1), n(0), r(nullptr), e(x) {
    Roar(&m, &n);
    m = InilineMethod();
  }

  void Method() const;

  int InilineMethod() {
    return (m + n) * r->Now();
  }
};

void Fooman::Method() const {
  ++m;
}

class Reman {
public:
  long Now() const {
    return ++bar;
  }

private:
  long bar;
};

}  // namespace YY
}  // namespace XX

#if defined(pex)
#pragma plex_test fixup 17 class laa::External
#pragma plex_test fixup 26 function Roar
#endif
