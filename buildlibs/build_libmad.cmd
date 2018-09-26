@echo on
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2018 Michael Fink
REM
REM Downloads libmad and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=libmad-0.15.1b

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community

REM download package
set URL=https://downloads.sourceforge.net/project/mad/libmad/0.15.1b/libmad-0.15.1b.tar.gz?r=https%%3A%%2F%%2Fsourceforge.net%%2Fprojects%%2Fmad%%2Ffiles%%2Flibmad%%2F0.15.1b%%2Flibmad-0.15.1b.tar.gz%%2Fdownload^&ts=1537981901

if not exist %PREFIX%.tar.gz powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.tar.gz}"

REM unzip
rmdir /s /q %PREFIX%\
del %PREFIX%.tar
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar.gz
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar
del %PREFIX%.tar

REM copy additional files
xcopy /s libmad-msvc\*.* %PREFIX%\

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\msvc++

REM compile
msbuild libmad.vcxproj /m /property:Configuration=Release,Platform=Win32

popd

REM copy artifacts
copy %PREFIX%\msvc++\Release\libmad.dll ..\source\libraries\
copy %PREFIX%\msvc++\Release\libmad.lib ..\source\libraries\lib\

pause
