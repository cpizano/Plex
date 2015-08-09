//#~def plx::GetLuid
///////////////////////////////////////////////////////////////////////////////
// plx::GetLuid
//
namespace plx {
uint64_t GetLuid() {
  LUID luid;
  ::AllocateLocallyUniqueId(&luid);
  ULARGE_INTEGER li = {luid.LowPart, luid.HighPart};
  return li.QuadPart;
}
}
