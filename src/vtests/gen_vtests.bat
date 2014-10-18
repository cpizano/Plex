@echo off
echo == plex vtest generator ==

set plexbin="..\plex\out\x64\%1\plex.exe"
if not exist %plexbin% goto error1

echo generating utests\ut_core.cpp
del ..\vtests\stdafx.cpp
del ..\vtests\stdafx.h
%plexbin% --generate --catalog=..\catalog --pch vtests.cpp

goto end

:error1
echo no plex binary found
echo %plexbin%
goto end

:end
echo == plex vtest generator end ==
