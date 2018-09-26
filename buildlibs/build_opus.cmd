@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2018 Michael Fink
REM
REM Downloads Opus and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=opus-1.3-rc2

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

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\win32\VS2015

REM update Toolset to v141, in order to use VS2017
powershell -Command "& {(Get-Content opus.vcxproj) -replace \"v140\",\"v141\" | out-file opus.vcxproj}"

REM compile
msbuild opus.vcxproj /m /property:Configuration=ReleaseDLL,Platform=Win32

popd

REM copy artifacts
mkdir include 2> nul
mkdir lib 2> nul
xcopy /Y %PREFIX%\include\*.h include\
xcopy /Y %PREFIX%\win32\VS2015\Win32\ReleaseDLL\*.lib lib\

copy %PREFIX%\win32\VS2015\Win32\ReleaseDLL\opus.dll ..\source\libraries\
copy %PREFIX%\win32\VS2015\Win32\ReleaseDLL\opus.lib ..\source\libraries\lib\
copy %PREFIX%\include\*.* ..\source\libraries\include\opus\

pause
