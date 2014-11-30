//#~def plx::JsonFromFile
///////////////////////////////////////////////////////////////////////////////
// plx::JsonFromFile.
namespace plx {
plx::JsonValue JsonFromFile(plx::File& cfile) {
  if (!cfile.is_valid())
    return false;
  auto size = cfile.size_in_bytes();
  if (size < 3)
    return false;
  plx::Range<uint8_t> r(0, plx::To<size_t>(size));
  auto mem = plx::HeapRange(r);
  if (cfile.read(r, 0) != size)
    return false;
  plx::Range<const char> json(reinterpret_cast<char*>(r.start()),
                              reinterpret_cast<char*>(r.end()));
  return plx::ParseJsonValue(json);
}
}