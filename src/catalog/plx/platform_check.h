//#~def plx::PlatformCheck
///////////////////////////////////////////////////////////////////////////////
// plx::PlatformCheck : checks if the code can be run in this setting.
//
namespace plx {
bool PlatformCheck() {
  __if_exists(plex_sse42_support) {
    if (!plx::CpuId().sse42())
      return false;
  }
  return true;
};
}
