// utests.h : unittests for the plex catalog.
//

const char hx[] = "0123456789ABCDEF";
#define ToHEX(x)  '0', 'x', hx[((x >> 12) & 0x0f)], hx[((x >> 8) & 0x0f)], hx[((x >> 4) & 0x0f)], hx[(x & 0x0f)]

class Test {
  static int run_count;
  static const char* test_name;

public:
  static int GetRunCount() { return run_count; }
  static const char* GetTestName() { return test_name; }

  Test(const char* name);

  ~Test();
  void Run();

private:
  virtual void Exec() = 0;
  long long tick_time_;

};

struct Fail {
  const char* kind;
  const char* test;

  Fail(const char* kind, const char* test) : kind(kind), test(test) {}
};

template<typename A>
void NotReached(A&&) {
  throw Fail("NR", Test::GetTestName());
}

template <typename A, typename B>
void CheckEQ(A&& a, B&& b, int ln) {
  if (a != b) {
    const char reason[] = { 'E','Q',' ', ToHEX(ln), 0 };
    throw Fail(reason, Test::GetTestName());
  }
}

template <typename A, typename B>
void CheckGT(A&& a, B&& b, int ln) {
  if (a <= b) {
    const char reason[] = { 'G','T',' ', ToHEX(ln), 0 };
    throw Fail(reason, Test::GetTestName());
  }
}

template <typename A, typename B>
void CheckLT(A&& a, B&& b, int ln) {
  if (a >= b) {
    const char reason[] = { 'L','T',' ', ToHEX(ln), 0 };
    throw Fail(reason, Test::GetTestName());
  }
}

#define CheckEQ(l, r) CheckEQ(l, r, __LINE__)
#define CheckGT(l, r) CheckGT(l, r, __LINE__)
#define CheckLT(l, r) CheckLT(l, r, __LINE__)

#define TEST(name) \
class name : public Test {\
public:\
  name() : Test(__FUNCTION__) {}\
private:\
  virtual void Exec() override;\
}

void InitGlobals();

TEST(Test_PlatformCheck);
TEST(Test_Range);
TEST(Test_BitSlice);
TEST(Test_CpuId);
TEST(Test_LinkedBuffers);
TEST(Test_To_Integer);
TEST(Test_ScopeGuard);
TEST(Test_Utf8decode);
TEST(Test_Utf8Utf16Conv);
TEST(Test_JsonValue);
TEST(Test_Hex);
TEST(Test_Whitespace);
TEST(Test_DecodeString);
TEST(Test_StringPrintf);
TEST(Test_Parse_JSON);
TEST(Test_CRC32C);
TEST(Test_FilePath);
TEST(Test_File);
TEST(Test_IOCPLoop);
TEST(Test_Hashes);
TEST(Test_CmdLine);
TEST(Test_Inflater);
TEST(Test_GZIP);
TEST(Test_ReaderWriterLock);
TEST(Test_Globals);
TEST(Test_VEHManager);
TEST(Test_DemandPagedMemory);
TEST(Test_ArgPack);
TEST(Test_RectLSizeL);
TEST(Test_SharedMemory);
TEST(Test_SharedBuffer);
#undef TEST
