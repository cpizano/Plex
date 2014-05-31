//#~def plx::ExePath
///////////////////////////////////////////////////////////////////////////////
// plx::ExePath
//
namespace plx {
plx::FilePath GetExePath() {
  wchar_t* pp = nullptr;
  _get_wpgmptr(&pp);
  return FilePath(pp).parent();
}
}
