@echo off
echo == plex unittest generator ==

set plexbin="..\plex\out\x64\%1\plex.exe"
if not exist %plexbin% goto error1

echo generating utests\ut_core.cpp
del ..\utests\stdafx.cpp
del ..\utests\stdafx.h
%plexbin% --generate --catalog=..\catalog --pch ut_core.cpp

goto end

:error1
echo no plex binary found
echo %plexbin%
goto end

:end
echo == plex unittest generator end ==
