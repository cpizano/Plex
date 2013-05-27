// test_006, part of the plex test suite.

#if defined(pex)
#pragma plex_test xdef class laa::External
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

  Fooman(External& x) 
      : m(1), n(0), r(nullptr), e(x) {
    Roar(&m, &n);
  }

  void Method() const;

  int InilineMethod() {
    return (m + n);
  }

};

void Fooman::Method() const {
  ++m;
}

class Reman {
  long bar;
};

}  // namespace YY
}  // namespace XX
