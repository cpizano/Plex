//#~def plx::StringPrintf
///////////////////////////////////////////////////////////////////////////////
// plx::StringPrintf  (c-style printf for std strings)
//
namespace plx {

namespace impl_v {
int vsnprintf(char* buffer, size_t size,
              const char* format, va_list arguments) {
  int length = _vsprintf_p(buffer, size, format, arguments);
  if (length < 0) {
    if (size > 0)
      buffer[0] = 0;
    return _vscprintf_p(format, arguments);
  }
  return length;
}
}

std::string StringPrintf(const char* fmt, ...) {
  int fmt_size = 128;
  std::unique_ptr<char> mem;

  va_list args;
  va_start(args, fmt);

  while (true) {
    mem.reset(new char[fmt_size]);
    int sz = impl_v::vsnprintf(mem.get(), fmt_size, fmt, args);
    if (sz < fmt_size)
      break;
    fmt_size = sz + 1;
  }

  va_end(args);
  return std::string(mem.get());
}

}
