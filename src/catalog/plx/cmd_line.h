//#~def plx::CmdLine
///////////////////////////////////////////////////////////////////////////////
// plx::CmdLine (handles command line arguments)
// 
namespace plx {

class CmdLine {
  
  struct KeyHash {
    size_t operator()(const plx::Range<const wchar_t>& r) const {
      return plx::Hash_FNV1a_64(r.const_bytes());
    }
  };

  struct KeyEqual {
    bool operator()(const plx::Range<const wchar_t>& lhs, const plx::Range<const wchar_t>& rhs) const {
      return lhs.equals(rhs);
    }
  };

  std::unordered_map<plx::Range<const wchar_t>, plx::Range<const wchar_t>, KeyHash, KeyEqual> opts_;
  std::vector<plx::Range<const wchar_t>> extra_;
  plx::Range<const wchar_t> program_;

public:
  CmdLine(int argc, wchar_t* argv[]) {
    if (!argc)
      return;

    int start;
    auto arg0 = plx::RangeUntilValue<wchar_t>(argv[0], 0);
    if (is_program(arg0)) {
      program_ = arg0;
      start = 1;
    } else {
      start = 0;
    }

    for (int ix = start; ix != argc; ++ix) {
      auto c_arg = plx::RangeUntilValue<wchar_t>(argv[ix], 0);
      if (IsOption(c_arg)) {
        c_arg.advance(2);
        opts_.insert(NameValue(c_arg));
      } else {
        extra_.push_back(c_arg);
      }
    }
  }

  template <size_t count>
  const bool has_switch(const wchar_t (&str)[count],
                        plx::Range<const wchar_t>* value = nullptr) const {

    return has_switch(plx::RangeFromLitStr<const wchar_t, count>(str), value);
  }

  const bool has_switch(const plx::Range<const wchar_t>& name,
                        plx::Range<const wchar_t>* value = nullptr) const {
    auto pos = opts_.find(name);
    bool found = pos != end(opts_);
    if (value && found) {
      *value = pos->second;
      return true;
    }
    return found;
  }

  size_t extra_count() const {
    return extra_.size();
  }

  plx::Range<const wchar_t> extra(size_t index) const {
    if (index >= extra_.size())
      return plx::Range<wchar_t>();
    return extra_[index];
  }

private:
  bool IsOption(const plx::Range<wchar_t>& r) {
    if (r.size() < 3)
      return false;
    return ((r[0] == '-') && (r[1] == '-') && (r[2] != '-') && (r[2] != '='));
  }

  bool is_program(const plx::Range<wchar_t>& r) const {
    // $$$ todo.
    return false;
  }

  std::pair<plx::Range<wchar_t>, plx::Range<wchar_t>> NameValue(plx::Range<wchar_t>& r) {
    size_t pos = 0;
    if (r.contains(L'=', &pos) && pos < (r.size() - 1))
      return std::make_pair(
          plx::Range<wchar_t>(r.start(), r.start() + pos),
          plx::Range<wchar_t>(r.start() + pos + 1, r.end()));
    return std::make_pair(r, plx::Range<wchar_t>());
  }
};

}