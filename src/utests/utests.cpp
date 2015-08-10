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
    InitGlobals();
    Test_PlatformCheck().Run();
    Test_Range().Run();
    Test_BitSlice().Run();
    Test_CpuId().Run();
    Test_LinkedBuffers().Run();
    Test_To_Integer().Run();
    Test_ScopeGuard().Run();
    Test_Utf8decode().Run();
    Test_Utf8Utf16Conv().Run();
    Test_JsonValue().Run();
    Test_Hex().Run();
    Test_Whitespace().Run();
    Test_DecodeString().Run();
    Test_StringPrintf().Run();
    Test_Parse_JSON().Run();
    Test_CRC32C().Run();
    Test_FilePath().Run();
    Test_File().Run();
    Test_IOCPLoop().Run();
    Test_Hashes().Run();
    Test_CmdLine().Run();
    Test_Inflater().Run();
    Test_GZIP().Run();
    Test_ReaderWriterLock().Run();
    Test_Globals().Run();
    Test_VEHManager().Run();
    Test_DemandPagedMemory().Run();
    Test_ArgPack().Run();
    Test_RectLSizeL().Run();
    Test_SharedMemory().Run();
    Test_LUID().Run();
    Test_SharedBuffer().Run();
  } catch (Fail& ex) {
    wprintf(L"Test %S  (%S) failed\n", ex.test, ex.kind);
    return 1;
  }

  wprintf(L"SUCCESS: %d test ran\n", Test::GetRunCount());
	return 0;
}

