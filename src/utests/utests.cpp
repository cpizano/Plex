// utests.cpp : unittests for the plex catalog.

#include "utests.h"

#include <SDKDDKVer.h>
#include <Windows.h>

#include <stdio.h>
#include <tchar.h>

int Test::run_count = 0;
const char* Test::test_name = nullptr;

Test::Test(const char* name) : tick_time_(0) {
  test_name = name;
  wprintf(L"[running] %S \n", test_name);
}

Test::~Test() {
  wprintf(L"[done] %d ms\n", tick_time_);
}

void Test::Run() {
  auto st = ::GetTickCount64();
  Exec();
  tick_time_ = ::GetTickCount64() - st;
  ++run_count;
}

int wmain(int argc, wchar_t* argv[]) {
  try {
    Test_Range().Run();
    Test_CpuId().Run();
    Test_To_Integer().Run();
    Test_ScopeGuard().Run();
    Test_Utf8decode().Run();
    Test_JsonValue().Run();
    Test_Hex().Run();
    Test_Whitespace().Run();

  } catch (Fail& ex) {
    wprintf(L"Test %S  (%S) failed\n", ex.test, ex.kind);
    return 1;
  }

  wprintf(L"SUCCESS: %d test ran\n", Test::GetRunCount());
	return 0;
}

