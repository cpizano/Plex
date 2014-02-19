@echo off
echo == plex test harness v1 ==

set plexbin="..\plex\out\x64\Release\plex.exe"
if not exist %plexbin% goto error1

set /a errcount=0
set /a count=0
setlocal ENABLEDELAYEDEXPANSION

for /F %%x in ('dir /B/D *.cc') do (
  echo processing %%x
  set /a count=count+1
  %plexbin% --dump-tree %%x
  if errorlevel 1 set /a errcount=errcount+1
)
echo .
echo !count! files processed.
echo .
if !errcount! NEQ 0 goto error2

set /a errcount=0
set /a count=0

for /F %%y in ('dir /B/D *.dmp') do (
  set /a count=count+1
  FC reference\%%y  %%y
  if errorlevel 1 set /a errcount=errcount+1
)
echo !count! files compared.
echo .
if !errcount! NEQ 0 goto error2

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
endlocal
