//#~def plx::GetAppDataPath
///////////////////////////////////////////////////////////////////////////////
// plx::GetAppDataPath
//
namespace plx {
plx::FilePath GetAppDataPath(bool roaming) {
  auto folder = roaming? FOLDERID_RoamingAppData : FOLDERID_LocalAppData;
  wchar_t* path = nullptr;
  auto hr = ::SHGetKnownFolderPath(folder, 0, nullptr, &path);
  if (hr != S_OK)
    throw plx::IOException(__LINE__, L"<appdata folder>");
  auto fp = FilePath(path);
  ::CoTaskMemFree(path);
  return fp;
}
}
