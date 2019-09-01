@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2019 Michael Fink
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
copy %ROOT%\bin\Release\*.chm zip\
copy %ROOT%\source\presets.xml zip\
copy %ROOT%\readme.txt zip\Readme.txt
copy %ROOT%\Copying zip\Copying.txt
copy "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.22.27821\vcredist_x86.exe" zip\vcredist_x86_vc142.exe

set ZIP="%ProgramFiles%\7-Zip\7z.exe"

cd zip
%ZIP% a ..\..\winLAME-zip.zip *.*
cd ..\..\..

set ZIP=
set ROOT=
