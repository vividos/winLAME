@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2026 Michael Fink
REM
REM Copies libraries to bin folder
REM

REM used when double-clicking
if "%1" == "" call CopyLibraries.cmd Debug
if "%1" == "" call CopyLibraries.cmd Release
if "%1" == "" exit 0

mkdir ..\..\bin 2> nul

if "%1" == "Debug" set VCPKG_BIN_DIR=%CD%\..\..\intermediate\vcpkg_installed\x86-windows\debug\bin
if "%1" == "Release" set VCPKG_BIN_DIR=%CD%\..\..\intermediate\vcpkg_installed\x86-windows\bin

set TARGET=..\..\bin\%1\Win32

mkdir %TARGET% 2> nul

copy bin\Win32\bass.dll %TARGET%
copy bin\Win32\basscd.dll %TARGET%
copy bin\Win32\basswma.dll %TARGET%
copy bin\Win32\libmp3lame.dll %TARGET%
copy bin\Win32\libfaac_dll.dll %TARGET%
copy bin\Win32\MACDll.dll %TARGET%

copy %VCPKG_BIN_DIR%\faad-2.dll %TARGET%

mkdir %ProgramData%\winLAME 2> nul
copy ..\presets.xml %ProgramData%\winLAME\presets.xml
