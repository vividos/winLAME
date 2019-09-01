@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2019 Michael Fink
REM
REM Downloads opusfile and compiles it
REM

REM Prerequisites: Compile libogg and opus
cmd /c "build_ogg.cmd"
cmd /c "build_opus.cmd"

REM set this to the filename of the file to download
set PREFIX=opusfile-0.11

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

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
xcopy include\*.h %PREFIX%\include\ogg\
xcopy include\*.h %PREFIX%\include\

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\win32\VS2015

REM update Toolset to v142, in order to use VS2019
powershell -Command "& {(Get-Content opusfile.vcxproj) -replace \"v140\",\"v142\" | out-file opusfile.vcxproj}"
powershell -Command "& {(Get-Content opusfile.vcxproj) -replace \"MultiThreaded\",\"MultiThreadedDLL\" | out-file opusfile.vcxproj}"

REM compile
msbuild opusfile.vcxproj /m /property:Configuration=Release-NoHTTP,Platform=Win32

popd

REM copy artifacts
copy %PREFIX%\win32\VS2015\Win32\Release-NoHTTP\opusfile.lib ..\source\libraries\lib\
copy %PREFIX%\win32\VS2015\Win32\Release-NoHTTP\opusfile\opusfile.pdb ..\source\libraries\lib\
copy %PREFIX%\include\opusfile.h ..\source\libraries\include\opus\

pause
