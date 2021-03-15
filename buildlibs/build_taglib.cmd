@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2021 Michael Fink
REM
REM Downloads taglib and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=taglib-1.12

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

REM download package
set URL=https://taglib.org/releases/%PREFIX%.tar.gz

if not exist %PREFIX%.tar.gz powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.tar.gz}"

REM unzip
rmdir /s /q %PREFIX%\
del %PREFIX%.tar 2> nul
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar.gz
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar
del %PREFIX%.tar 2> nul
del pax_global_header 2> nul

REM copy additional files
xcopy /s taglib-msvc\*.* %PREFIX%\

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\taglib

REM compile
msbuild taglib.sln /m /property:Configuration=Release
msbuild taglib.sln /m /property:Configuration=Debug

popd

REM copy artifacts
copy %PREFIX%\taglib\Release\taglib.dll ..\source\libraries\
copy %PREFIX%\taglib\Release\taglib.lib ..\source\libraries\lib\
copy %PREFIX%\taglib\Debug\taglib.dll ..\source\libraries\taglib_debug.dll
copy %PREFIX%\taglib\Debug\taglib.lib ..\source\libraries\lib\taglib_debug.lib
copy %PREFIX%\taglib\*.h ..\source\libraries\include\taglib\
xcopy /S /Y %PREFIX%\taglib\toolkit\*.h ..\source\libraries\include\taglib\toolkit\
xcopy /S /Y %PREFIX%\taglib\toolkit\*.tcc ..\source\libraries\include\taglib\toolkit\
xcopy /S /Y %PREFIX%\taglib\mpeg\*.h ..\source\libraries\include\taglib\mpeg\
xcopy /S /Y %PREFIX%\taglib\mpeg\id3v2\*.h ..\source\libraries\include\taglib\mpeg\id3v2\
xcopy /S /Y %PREFIX%\taglib\ogg\*.h ..\source\libraries\include\taglib\ogg\
xcopy /S /Y %PREFIX%\taglib\flac\*.h ..\source\libraries\include\taglib\flac\

pause
