// test_008, part of the plex test suite.


int wmain(int argc, wchar_t* argv[]) {
  if (argc > 2)
    throw plx::Exception(__LINE__, "missing args");

  plx::CpuId cpu_id;
  printf("= stepping %d model %d family %d\n",
         cpu_id.stepping(), cpu_id.model(), cpu_id.family());
  return 0;
}
