//#~def plx::ProcessCreationTime
///////////////////////////////////////////////////////////////////////////////
// plx::ProcessCreationTime.
//
namespace plx {
uint64_t ProcessCreationTime(HANDLE process) {
  FILETIME creat, exit, kernel, user;
  if (::GetProcessTimes(process, &creat, &exit, &kernel, &user) == 0)
    return 0ULL;
  LARGE_INTEGER li;
  li.HighPart = creat.dwHighDateTime;
  li.LowPart = creat.dwLowDateTime;
  return li.QuadPart;
}
}
