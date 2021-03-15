@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2021 Michael Fink
REM
REM Downloads LAME and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=lame-svn-r6505-trunk

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

REM download package
set URL=https://sourceforge.net/code-snapshots/svn/l/la/lame/svn/%PREFIX%.zip

if not exist %PREFIX%.zip powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.zip}"

REM unzip
rmdir /s /q %PREFIX%\
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.zip

REM also download mpg123
set URL=https://mpg123.de/download/win32/1.26.4/mpg123-1.26.4-x86.zip

if not exist mpg123-x86.zip powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out mpg123-x86.zip}"

REM unzip
rmdir /s /q %PREFIX%\lame\vc_solution\mpg123\ 2> nul
pushd %PREFIX%\lame\vc_solution\
"c:\Program Files\7-Zip\7z.exe" x ..\..\..\mpg123-x86.zip
mkdir mpg123
move mpg123-1.26.4-x86 mpg123\Win32
popd

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\lame\vc_solution

REM compile
REM msbuild vs2019_lame.sln /m /property:Configuration=Release /property:HaveMpg123=true
msbuild vs2019_libmp3lame_dll.vcxproj /m /property:Configuration=Release /property:HaveMpg123=true

popd

REM copy artifacts
copy %PREFIX%\lame\output\Win32\Release\libmp3lame.dll ..\source\libraries\
copy %PREFIX%\lame\output\Win32\Release\libmp3lame.lib ..\source\libraries\lib\
copy %PREFIX%\lame\include\*.h ..\source\libraries\include\lame\

pause
