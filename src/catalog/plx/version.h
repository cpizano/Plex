//#~def plx::Version
///////////////////////////////////////////////////////////////////////////////
// plx::Version
//
namespace plx {

class Version {
  struct V {
    uint16_t major;
    uint16_t minor;
    uint16_t rev;
    uint16_t build;
  } v;

public:
  Version() : v() {}

  Version(uint16_t major, uint16_t minor, uint16_t rev, uint16_t build)
    : v({ major, minor, rev, build }) {}

  std::string to_string() const {
    return plx::StringPrintf("%d.%d.%d.%d",
                             v.major, v.minor, v.rev, v.build);
  }

  int major() const { return v.major; }
  int minor() const { return v.minor; }
  int rev() const { return v.rev; }
  int build() const { return v.build; }

  static Version FromString(plx::Range<const uint8_t> r) {
    if (r.empty())
      throw plx::CodecException(__LINE__, nullptr);

    int ix = 0;
    uint16_t v[4] = {};
    while (true) {
      auto n = plx::ParseUnsignedInt<10>(r);
      v[ix] = plx::To<uint16_t>(n);
      if (ix == 3)
        break;
      if (r.empty())
        break;
      if (r.front() != '.')
        throw plx::CodecException(__LINE__, &r);
      r.advance(1);
      ++ix;
    }
    return Version(v[0], v[1], v[2], v[3]);
  }

  static int Compare(const Version& l, const Version& r) {
    int x = l.v.major - r.v.major;
    if (x)
      return x;
    x = l.v.minor - r.v.minor;
    if (x)
      return x;
    x = l.v.rev - r.v.rev;
    if (x)
      return x;
    x = l.v.build - r.v.build;
    return x;
  }

};

}
