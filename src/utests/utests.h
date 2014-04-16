// utests.h : unittests for the plex catalog.
//

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
void CheckEQ(A&& a, B&& b) {
  if (a != b)
    throw Fail("EQ", Test::GetTestName());
}

template <typename A, typename B>
void CheckGT(A&& a, B&& b) {
  if (a <= b)
    throw Fail("GT", Test::GetTestName());
}

template <typename A, typename B>
void CheckLT(A&& a, B&& b) {
  if (a >= b)
    throw Fail("LT", Test::GetTestName());
}

#define TEST(name) \
class name : public Test {\
public:\
  name() : Test(__FUNCTION__) {}\
private:\
  virtual void Exec() override;\
}

TEST(Test_Range);
TEST(Test_CpuId);
TEST(Test_To_Integer);
TEST(Test_ScopeGuard);
TEST(Test_Utf8decode);
TEST(Test_JsonValue);
TEST(Test_Hex);
TEST(Test_Whitespace);
TEST(Test_Parse_JSON);
#undef TEST
