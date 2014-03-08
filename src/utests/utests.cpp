// utests.cpp : unittests for the plex catalog.

#include "utests.h"

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>

int Test::run_count = 0;
const char* Test::test_name = nullptr;


int wmain(int argc, wchar_t* argv[]) {

  try {
    Test_Range().Run();
    Test_CpuId().Run();

  } catch (Fail& ex) {
    wprintf(L"Test %S of line %d failed\n", ex.test, ex.line);
    return 1;
  }

  wprintf(L"SUCCESS: %d test ran\n", Test::GetRunCount());
	return 0;
}

