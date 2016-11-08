@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2016 Michael Fink
REM
REM Creates a winLAME build and installer
REM

REM -{ config here }-----------------------------

set MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0

REM -{ config end }------------------------------

REM set up build environment
call "%MSVC_PATH%\Common7\Tools\VsDevCmd.bat"

REM build solution
msbuild /m:4 winlame.sln /property:Configuration=Release /target:Rebuild

call source\libraries\CopyLibraries.cmd Release

REM build winLAME Portable
call BuildPortable.cmd

REM build winLAME zip archive
call BuildZip.cmd

REM finished
echo Finished!

set MSVC_PATH=

pause
