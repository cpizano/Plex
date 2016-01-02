//#~def plx::StringPrintf
//#~def plx::vsnprintf
///////////////////////////////////////////////////////////////////////////////
// plx::StringPrintf  (c-style printf for std strings)
//
namespace plx {

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

int vsnprintf(wchar_t* buffer, size_t size,
              const wchar_t* format, va_list arguments) {
  int length = _vswprintf_p(buffer, size, format, arguments);
  if (length < 0) {
    if (size > 0)
      buffer[0] = 0;
    return _vscwprintf_p(format, arguments);
  }
  return length;
}


template <typename CH>
std::basic_string<CH> StringPrintf(const CH* fmt, ...) {
  int fmt_size = 128;
  std::unique_ptr<CH> mem;

  va_list args;
  va_start(args, fmt);

  while (true) {
    mem.reset(new CH[fmt_size]);
    int sz = vsnprintf(mem.get(), fmt_size, fmt, args);
    if (sz < fmt_size)
      break;
    fmt_size = sz + 1;
  }

  va_end(args);
  return mem.get();
}


}
