@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2016 Michael Fink
REM
REM BuildPortable.cmd - Builds winLAME Portable
REM

set ROOT=%CD%

cd bin\Release

rmdir /S /Q winLAMEPortable 2> nul

mkdir winLAMEPortable

xcopy /S %ROOT%\source\portable\*.* winLAMEPortable\

mkdir winLAMEPortable\App\winLAME
copy %ROOT%\bin\Release\*.dll winLAMEPortable\App\winLAME\
copy %ROOT%\bin\Release\*.exe winLAMEPortable\App\winLAME\

mkdir winLAMEPortable\Data
copy %ROOT%\source\presets.xml  winLAMEPortable\Data\

cd ..\..

set ROOT=
