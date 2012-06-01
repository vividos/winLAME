@echo off
REM Creates a winLAME build, HTML help and installer
REM

REM -{ config here }-----------------------------

set MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 9.0
set PLATFORM_SDK_PATH=C:\Program Files\Microsoft SDKs\Windows\v6.0a
set BOOST_PATH=D:\devel\packages\boost_1_47_0
set WIX_PATH=D:\devel\tools\wix-v3\bin
set DOXYGEN_PATH=D:\devel\tools\doxygen\bin

REM -{ config end }------------------------------

REM set up msvc8 env
call "%MSVC_PATH%\VC\vcvarsall.bat" x86

REM add ATL path
set INCLUDE=%INCLUDE%;%PLATFORM_SDK_PATH%\Include;%PLATFORM_SDK_PATH%\Include\atl
set LIB=%LIB%;%PLATFORM_SDK_PATH%\Lib

set INCLUDE=%INCLUDE%;%BOOST_PATH%

REM build solution
vcbuild /logfile:build-log.txt /M4 /nohtmllog /rebuild /time /useenv winlame.sln "Release|Win32"

REM finished
echo Finished!

set MSVC_PATH=
set PLATFORM_SDK_PATH=
set BOOST_PATH=
set WIX_PATH=
set DOXYGEN_PATH=

pause
