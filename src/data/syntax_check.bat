@echo off
echo == plex compiler syntax check v1 ==
call "%VS120COMNTOOLS%"vsvars32.bat
if errorlevel NEQ 0 goto error0

set plexbin="..\plex\out\x64\%1\plex.exe"
if not exist %plexbin% goto error2

REM put the ones that should compile here.
del full\test_007.cc 
%plexbin% --generate --out-dir=full test_007.cc
del full\test_008.cc 
%plexbin% --generate --out-dir=full test_008.cc

set /a errcount=0
set /a count=0
setlocal ENABLEDELAYEDEXPANSION

for /F %%x in ('dir /B/D full\*.cc') do (
  echo processing %%x
  set /a count=count+1
  cl.exe /Zs full\%%x
  if errorlevel 1 set /a errcount=errcount+1
)
echo .
echo !count! files processed.
echo .
if !errcount! NEQ 0 goto error1

echo no errors found.
goto end

:error0
echo missing toolchain VS2013.
goto end

:error1
echo %errcount% file(s) failed.
goto end

:error2
echo missing plex tool
goto end

:end
echo == plex syntax check end ==
endlocal


