@echo off
REM Creates a winLAME build and installer
REM

REM -{ config here }-----------------------------

set MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0

REM -{ config end }------------------------------

REM set up msvc14 environment
call "%MSVC_PATH%\Common7\Tools\VsDevCmd.bat"

REM build solution
msbuild /m:4 winlame.sln /property:Configuration=Release /target:Rebuild

REM build winLAME Portable
call BuildPortable.cmd

REM finished
echo Finished!

set MSVC_PATH=

pause
