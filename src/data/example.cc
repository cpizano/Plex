// The copyright goes here
// and here.

#if 1
namespace plex {
  class Foo {
  public :
    bool is_prime() const { return true; }
    float value() const { return 33.0; }
  };
}
#else

#endif

long global_dec = -778;  
long goblal_hex = 0xB671;

const char line_feed = '\n';
const char name[] = "fooo bar";
const wchar_t the_surname[] = L"soft \"masato";
const double dable = .33335671e-12;

typedef float V;

// The bar function.
V bar(V x, V y, int z) {
  if (--z >= 3)
    return x + y * (z - 1);
  else
    return y + z;
}

// The foo function.
V foo (const plex::Foo& f, V y) {
  if (f.is_prime() && (int(y) & 8) )
    return 0.0;    // TODO(cpu)
  return bar(f.value(), y, 0);
}

// The baz function.
bool baz(unsigned int pp, char* arr) {
  const float lema = (pp == 3)? 3.0f : 12.11123f;
  global_dec += arr[1];
  return float(arr[0]++) != lema;
}
