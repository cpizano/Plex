@echo off
echo == plex unittest generator ==

set plexbin="..\plex\out\x64\%1\plex.exe"
if not exist %plexbin% goto error1

echo generating utests\ut_core.cpp
del ..\utests\gen\ut_core.cpp
%plexbin% --generate --out-dir=gen --catalog=..\catalog ut_core.cpp
goto end

:error1
echo no plex binary found
echo %plexbin%
goto end

:end
echo == plex unittest generator end ==
