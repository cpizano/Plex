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

