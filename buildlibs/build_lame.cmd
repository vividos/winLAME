@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2020 Michael Fink
REM
REM Downloads LAME and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=lame-3.100

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

REM download package
set URL=https://TODO/%PREFIX%.tar.gz

if not exist %PREFIX%.tar.gz powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.tar.gz}"

REM unzip
rmdir /s /q %PREFIX%\
del %PREFIX%.tar 2> nul
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar.gz
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar
del %PREFIX%.tar 2> nul

REM also download mpg123
set URL=https://mpg123.de/download/win32/1.26.3/mpg123-1.26.3-x86.zip

if not exist mpg123-x86.zip powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out mpg123-x86.zip}"

REM unzip
rmdir /s /q %PREFIX%\vc_solution\mpg123\ 2> nul
pushd %PREFIX%\vc_solution\
"c:\Program Files\7-Zip\7z.exe" x ..\..\mpg123-x86.zip
move mpg123-1.26.3-x86 mpg123
popd

REM copy additional files
xcopy /s lame-msvc\*.* %PREFIX%\

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\vc_solution

REM compile
REM msbuild vs2019_lame.sln /m /property:Configuration=Release /property:HaveMpg123=true
msbuild vs2019_libmp3lame_dll.vcxproj /m /property:Configuration=Release /property:HaveMpg123=true

popd

REM copy artifacts
copy %PREFIX%\output\Release\libmp3lame.dll ..\source\libraries\
copy %PREFIX%\output\Release\libmp3lame.lib ..\source\libraries\lib\
copy %PREFIX%\include\*.h ..\source\libraries\include\lame\

pause
