@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2023 Michael Fink
REM
REM Copies libraries to bin folder
REM

REM used when double-clicking
if "%1" == "" call CopyLibraries.cmd Debug
if "%1" == "" call CopyLibraries.cmd Release
if "%1" == "" exit 0

mkdir ..\..\bin 2> nul

if "%1" == "Debug" set VCPKG_BIN_DIR=%CD%\..\..\vcpkg_installed\x86-windows\x86-windows\Debug\bin
if "%1" == "Release" set VCPKG_BIN_DIR=%CD%\..\..\vcpkg_installed\x86-windows\x86-windows\bin

set TARGET=..\..\bin\%1

mkdir %TARGET% 2> nul

copy bass.dll %TARGET%
copy basscd.dll %TARGET%
copy basswma.dll %TARGET%
copy libmp3lame.dll %TARGET%
copy libfaac_dll.dll %TARGET%
copy %VCPKG_BIN_DIR%\MACDll.dll %TARGET%

mkdir %ProgramData%\winLAME 2> nul
copy ..\presets.xml %ProgramData%\winLAME\presets.xml
