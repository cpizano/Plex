// test_006, part of the plex test suite.

namespace XX {
namespace YY {

class Fooman {
  int m;
  int n;
  Reman* r;
  External e;

public:
  Fooman(const Reman& rr) 
      : m(0), n(1), r(new Reman(rr)), e(0) {
  }

  Fooman(External& x) 
      : m(1), n(0), r(nullptr), e(x) {
    Roar(&m, &n);
  }

  void Method() const;

};

void Fooman::Method() const {
  ++m;
}

class Reman {
  long bar;
};

}  // namespace YY
}  // namespace XX
