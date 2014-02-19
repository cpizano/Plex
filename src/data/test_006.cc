// test_006, part of the plex test suite.
#pragma comment(user, "plex.arch=AMDx64")

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

