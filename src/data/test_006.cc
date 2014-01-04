// test_006, part of the plex test suite.

namespace xx {
namespace yy {

class Reman;

class Fooman {
  int m;
  int n;
  Reman* r;
  pxx::TestClassA e;

public:
  Fooman(const Reman& rr) 
      : m(0), n(1), r(new Reman(rr)), e(0) {
  }

  Fooman(laa::External& x) 
      : m(1), n(0), r(nullptr), e(x) {
    pxx::TestFunA(&m, &n);
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
  std::string name;
};

}  // namespace YY
}  // namespace XX

#if defined(pex)
#pragma plex_test fixup 12 cs pxx::TestClassA
#pragma plex_test fixup 21 fn pxx::TestFunA
#pragma plex_test fixup 44 in std::string
#endif
