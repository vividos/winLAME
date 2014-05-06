@echo off
REM Creates a winLAME build, HTML help and installer
REM

REM -{ config here }-----------------------------

set MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0

REM -{ config end }------------------------------

REM set up msvc12 env
call "%MSVC_PATH%\Common7\Tools\VsDevCmd.bat"

REM build solution
msbuild /m:4 winlame.sln /property:Configuration=Release /target:Rebuild

REM finished
echo Finished!

set MSVC_PATH=

pause
