@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2026 Michael Fink
REM
REM Downloads LAME and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=lame-4.0

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles%\Microsoft Visual Studio\18\Community

REM download package
set URL=https://sourceforge.net/projects/lame/files/lame/4.0/lame-4.0.tar.gz/download?use_mirror=deac-fra

if not exist %PREFIX%.tar.gz (
	powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.tar.gz}"
)

REM unzip
rmdir /s /q %PREFIX%\ 2> nul
del %PREFIX%.tar 2> nul
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar.gz
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar

REM also download mpg123
set URL=https://mpg123.de/download/win32/1.33.5/mpg123-1.33.5-x86.zip

if not exist mpg123-x86.zip (
	powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out mpg123-x86.zip}"
)

REM unzip
rmdir /s /q %PREFIX%\vc_solution\mpg123\ 2> nul
pushd %PREFIX%\vc_solution\
"c:\Program Files\7-Zip\7z.exe" x ..\..\mpg123-x86.zip
mkdir mpg123
move mpg123-1.33.5-x86 mpg123\Win32
popd

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\vc_solution

REM update Toolset to v145, in order to use VS2026
powershell -Command "& {(Get-Content vs2019_libmp3lame.vcxproj) -replace \"v142\",\"v145\" | out-file vs2019_libmp3lame.vcxproj}"
powershell -Command "& {(Get-Content vs2019_libmp3lame_dll.vcxproj) -replace \"v142\",\"v145\" | out-file vs2019_libmp3lame_dll.vcxproj}"

REM vcpkg builds an mpg123.dll, but the libmp3lame projects expect a libmpg123-0.dll; adjust the .def file for that
copy mpg123\Win32\libmpg123-0.def mpg123\Win32\mpg123.def
copy mpg123\Win32\libmpg123-0.dll mpg123\Win32\mpg123.dll
powershell -Command "& {(Get-Content vs2019_libmpg123_config.props) -replace \"libmpg123-0\",\"mpg123\" | out-file vs2019_libmpg123_config.props}"

REM compile
msbuild vs2019_libmp3lame_dll.vcxproj /m /property:Configuration=Release /property:HaveMpg123=true

popd

REM copy artifacts
copy %PREFIX%\output\Win32\Release\libmp3lame.dll ..\source\libraries\bin\Win32\
copy %PREFIX%\output\Win32\Release\libmp3lame.lib ..\source\libraries\lib\
copy %PREFIX%\include\*.h ..\source\libraries\include\lame\

pause
