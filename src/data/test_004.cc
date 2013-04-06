// test_004, part of the plex test suite.

bool ProcessTestPragma(MVector::iterator it,
                       const MVector& tokens) 
{
  // Some smart comment here.
  std::istringstream ss((it + 1)->range.Start());
  if (EqualToStr(*it, "token_count")) {
    long line, expected_count;
    ss >> line >> expected_count;
    if (!ss) {
      ::exception_seen = true;
      throw FastException(__LINE__, it->line);
    }
  return (expected_count == tokens.size());
}

extern int lamda::Royal::bee = 1;

#if defined(_WIN64) && defined(_MT)
// Ranting here.
lamda::Royal::bee = 2;
#endif

#if defined(plex)
#pragma plex_test token_count 3 6
#pragma plex_test token_count 4 5
#pragma plex_test token_count 5 1
#pragma plex_test token_count 6 1
#pragma plex_test token_count 7 16  // note: -> is two tokens.
#pragma plex_test token_count 8 11
#pragma plex_test token_count 9 5
#pragma plex_test token_count 10 6
#pragma plex_test token_count 11 6
#pragma plex_test token_count 12 4
#pragma plex_test token_count 13 11
#pragma plex_test token_count 14 1
#pragma plex_test token_count 15 11
#pragma plex_test token_count 16 1
#pragma plex_test token_count 18 6
#pragma plex_test token_count 19 0
#pragma plex_test token_count 20 1
#pragma plex_test name_count 27
#endif
