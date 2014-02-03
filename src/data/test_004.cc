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

