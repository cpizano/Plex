// test_007, part of the plex test suite.

int wmain(int argc, wchar_t* argv[]) {
  throw plx::Exception(__LINE_, "Whoa!");
	return 0;
}
