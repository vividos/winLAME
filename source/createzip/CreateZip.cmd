@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2026 Michael Fink
REM
REM BuildZip.cmd - Builds winLAME zip redist
REM

set ROOT=%CD%\..\..\
set CONFIG=%1
set PLATFORM=Win32

pushd "%CD%\..\libraries\"
call CopyLibraries.cmd %CONFIG%
popd

pushd ..\..\bin\%CONFIG%\%PLATFORM%

rmdir /S /Q zip 2> nul
del ..\..\winLAME-zip.zip 2> nul
del ..\..\winLAME-%appveyor_build_version%.zip 2> nul

mkdir zip

copy %ROOT%\bin\%CONFIG%\%PLATFORM%\*.dll zip\
del zip\unittest.dll
copy %ROOT%\bin\%CONFIG%\%PLATFORM%\*.exe zip\
copy %ROOT%\bin\%CONFIG%\%PLATFORM%\*.chm zip\
copy %ROOT%\source\presets.xml zip\
copy %ROOT%\source\setup\readme.txt zip\Readme.txt
copy %ROOT%\Copying zip\Copying.txt
copy "%VCToolsRedistDir%\vc_redist.x86.exe" zip\vcredist_x86_vc143.exe

set ZIP="%ProgramW6432%\7-Zip\7z.exe"
set ZIPFILENAME=winLAME-zip.zip
if not "%appveyor_build_version%" == "" set ZIPFILENAME=winLAME-%appveyor_build_version%.zip

cd zip
%ZIP% a ..\%ZIPFILENAME% *.*

set ZIP=
set ROOT=
