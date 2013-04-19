// test_005, part of the plex test suite.

#include <stdio.h>
#include <tchar.h>
#include <string>

std::string UTF16ToAscii(const std::wstring& utf16) {
  std::string result;
  result.reserve(utf16.size());
  for (auto it = utf16.begin(); it != utf16.end(); ++it) {
    if ((*it) <= 0xFF) {
      result.append(1, static_cast<char>(*it));
    }
  }
  return result;
}

#if defined(plex)
#pragma plex_test token_count 3 1
#pragma plex_test token_count 4 1
#pragma plex_test token_count 5 1
#pragma plex_test token_count 7 9
#pragma plex_test token_count 8 3
#pragma plex_test token_count 9 11
#pragma plex_test token_count 10 23
#pragma plex_test token_count 11 10
#pragma plex_test token_count 12 16
#pragma plex_test token_count 13 1
#pragma plex_test token_count 14 1
#pragma plex_test token_count 15 3
#pragma plex_test token_count 16 1
#pragma plex_test name_count 22
#endif
