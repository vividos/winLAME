@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2016 Michael Fink
REM
REM BuildZip.cmd - Builds winLAME zip redist
REM

set ROOT=%CD%

cd bin\Release

rmdir /S /Q zip 2> nul
del ..\winLAME-zip.zip

mkdir zip

copy %ROOT%\bin\Release\*.dll zip\
copy %ROOT%\bin\Release\*.exe zip\
copy %ROOT%\source\presets.xml zip\
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\redist\1033\vcredist_x86.exe" zip\vcredist_x86_vc14.exe

set ZIP="%ProgramFiles%\7-Zip\7z.exe"

cd zip
%ZIP% a ..\..\winLAME-zip.zip *.*
cd ..\..\..

set ZIP=
set ROOT=
