// Some comment here

long global_dec = -778;  
long goblal_hex = 0xB671;

const char line_feed = '\n';
const char name[] = "fooo bar";
const wchar_t the_surname[] = L"soft \"masato";
const double dable = .33335671e-12;

#pragma plex_test token_count 1 1
#pragma plex_test token_count 3 5
#pragma plex_test token_count 4 5
#pragma plex_test token_count 5 0
#pragma plex_test token_count 6 6
#pragma plex_test token_count 7 8
#pragma plex_test token_count 8 8
#pragma plex_test token_count 9 6
#pragma plex_test token_count 10 0
#pragma plex_test name_count 6
