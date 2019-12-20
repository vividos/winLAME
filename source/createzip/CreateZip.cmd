@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2019 Michael Fink
REM
REM BuildZip.cmd - Builds winLAME zip redist
REM

set ROOT=%CD%\..\..\
set CONFIG=%1

call "%CD%\..\libraries\CopyLibraries.cmd" %CONFIG%

pushd ..\..\bin\%CONFIG%

rmdir /S /Q zip 2> nul
del ..\winLAME-zip.zip 2> nul
del ..\winLAME-%appveyor_build_version%.zip 2> nul

mkdir zip

copy %ROOT%\bin\%CONFIG%\*.dll zip\
del zip\unittest.dll
copy %ROOT%\bin\%CONFIG%\*.exe zip\
copy %ROOT%\bin\%CONFIG%\*.chm zip\
copy %ROOT%\source\presets.xml zip\
copy %ROOT%\source\setup\readme.txt zip\Readme.txt
copy %ROOT%\Copying zip\Copying.txt
copy "%VCToolsRedistDir%\vcredist_x86.exe" zip\vcredist_x86_vc142.exe

set ZIP="%ProgramW6432%\7-Zip\7z.exe"
set ZIPFILENAME=winLAME-zip.zip
if not "%appveyor_build_version%" == "" set ZIPFILENAME=winLAME-%appveyor_build_version%.zip

cd zip
%ZIP% a ..\%ZIPFILENAME% *.*

set ZIP=
set ROOT=
