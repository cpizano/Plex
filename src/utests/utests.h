// utests.h : unittests for the plex catalog.
//


class Test {
  static int run_count;
  static const char* test_name;

public:
  static int GetRunCount() { return run_count; }
  static const char* GetTestName() { return test_name; }

  Test(const char* name) {
    test_name = name;
  }

  void Run() {
    Exec();
    ++run_count;
  }

private:
  virtual void Exec() = 0;

};

struct Fail {
  int line;
  const char* test;

  Fail(int line, const char* test) : line(line), test(test) {}
};

template <typename A, typename B>
void CheckEQ(A&& a, B&& b) {
  if (a != b)
    throw Fail(__LINE__, Test::GetTestName());
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
