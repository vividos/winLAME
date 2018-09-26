@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2018 Michael Fink
REM
REM Downloads libogg and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=libogg-1.3.3

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community

REM download package
set URL=https://downloads.xiph.org/releases/ogg/%PREFIX%.tar.gz

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
powershell -Command "& {(Get-Content libogg_dynamic.vcxproj) -replace \"v120\",\"v141\" | out-file libogg_dynamic.vcxproj}"

REM compile
msbuild libogg_dynamic.vcxproj /m /property:Configuration=Release,Platform=Win32

popd

REM copy artifacts
mkdir include 2> nul
xcopy /Y %PREFIX%\include\ogg\*.h include\

copy %PREFIX%\win32\VS2015\Win32\Release\libogg.dll ..\source\libraries\
copy %PREFIX%\win32\VS2015\Win32\Release\libogg.lib ..\source\libraries\lib\
copy %PREFIX%\include\ogg\*.h ..\source\libraries\include\ogg\

pause
