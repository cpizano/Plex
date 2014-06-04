//#~def plx::GetExePath
///////////////////////////////////////////////////////////////////////////////
// plx::GetExePath
//
namespace plx {
plx::FilePath GetExePath() {
  wchar_t* pp = nullptr;
  _get_wpgmptr(&pp);
  return FilePath(pp).parent();
}
}
