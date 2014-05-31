//#~def plx::LocalUniqueId
///////////////////////////////////////////////////////////////////////////////
// plx::LocalUniqueId.
// requres advapi32.lib
//
namespace plx {
uint64_t LocalUniqueId() {
  LUID luid = {0};
  ::AllocateLocallyUniqueId(&luid);
  ULARGE_INTEGER li = {luid.LowPart, luid.HighPart};
  return li.QuadPart;
}
}
