//#~def plx::LocalUniqueId
///////////////////////////////////////////////////////////////////////////////
// plx::LocalUniqueId.
// requres advapi32.lib
//
namespace plx {
uint64_t LocalUniqueId() {
  LUID luid;
  ::AllocateLocallyUniqueId(&luid);
  LARGE_INTEGER li = {luid.LowPart, luid.HighPart};
  return li.QuadPart;
}
}
