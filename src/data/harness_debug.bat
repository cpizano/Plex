@echo off
echo == plex test harness v1 ==

set plexbin="..\plex\out\x64\Debug\plex.exe"
if not exist %plexbin% goto error1

set /a errcount=0
set /a count=0

for /F %%x in ('dir /B/D *.cc') do (
  echo processing %%x
  %plexbin% --tokens-test %%x
  if errorlevel 1 set /a errcount+=1
  set /a count+=1
)

echo %count% files processed.

if %errcount% NEQ 0 goto error2
echo no errors found.
goto end

:error1
echo no plex binary found
echo %plexbin%
goto end

:error2
echo %errcount% test(s) failed.
goto end

:end
echo == plex test harness end ==
