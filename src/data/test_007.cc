// test_007, part of the plex test suite.

int wmain(int argc, wchar_t* argv[]) {
  if (argc > 3)
    throw plx::IOException(__LINE__);

  return 0;
}
