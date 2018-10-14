@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2018 Michael Fink
REM
REM Downloads libopusenc and compiles it
REM

REM Prerequisites: Compile libogg and opus
cmd /c "build_ogg.cmd"
cmd /c "build_opus.cmd"

REM set this to the filename of the file to download
set PREFIX=libopusenc-0.2.1

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community

REM download package
set URL=https://downloads.xiph.org/releases/opus/%PREFIX%.tar.gz

if not exist %PREFIX%.tar.gz powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.tar.gz}"

REM unzip
rmdir /s /q %PREFIX%\
del %PREFIX%.tar
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar.gz
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar
del %PREFIX%.tar

REM Prerequisite: Copy header files from libogg and opus
xcopy include\*.h %PREFIX%\include\
xcopy lib\*.lib %PREFIX%\win32\VS2015\

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\win32\VS2015

REM update Toolset to v141, in order to use VS2017
powershell -Command "& {(Get-Content opusenc.vcxproj) -replace \"v140\",\"v141\" | out-file opusenc.vcxproj}"

REM compile
msbuild opusenc.vcxproj /m /property:Configuration=ReleaseDLL,Platform=Win32

popd

REM copy artifacts
copy %PREFIX%\win32\VS2015\Win32\ReleaseDLL\opusenc.dll ..\source\libraries\
copy %PREFIX%\win32\VS2015\Win32\ReleaseDLL\opusenc.lib ..\source\libraries\lib\
copy %PREFIX%\include\opusenc.h ..\source\libraries\include\opus\

pause
