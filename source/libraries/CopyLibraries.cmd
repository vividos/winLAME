@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2020 Michael Fink
REM
REM Copies libraries to bin folder
REM

REM used when double-clicking
if "%1" == "" call CopyLibraries.cmd Debug
if "%1" == "" call CopyLibraries.cmd Release
if "%1" == "" exit 0

mkdir ..\..\bin 2> nul

set TARGET=..\..\bin\%1

mkdir %TARGET% 2> nul

copy bass.dll %TARGET%
copy basscd.dll %TARGET%
copy basswma.dll %TARGET%
copy libFLAC_dynamic.dll %TARGET%
copy libmp3lame.dll %TARGET%
copy sndfile.dll %TARGET%
copy libvorbis.dll %TARGET%
copy libvorbisfile.dll %TARGET%
copy libspeex.dll %TARGET%
copy opus.dll %TARGET%
copy opusenc.dll %TARGET%
copy ogg.dll %TARGET%
copy libfaac_dll.dll %TARGET%
copy libfaad2_dll.dll %TARGET%
copy MACDll.dll %TARGET%
copy libmpg123-0.dll %TARGET%
if "%1" == "Release" copy taglib.dll %TARGET%
if "%1" == "Debug" copy taglib_debug.dll %TARGET%\taglib.dll

mkdir %ProgramData%\winLAME 2> nul
copy ..\presets.xml %ProgramData%\winLAME\presets.xml
