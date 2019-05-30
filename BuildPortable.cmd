@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2019 Michael Fink
REM
REM BuildPortable.cmd - Builds winLAME Portable
REM

set ROOT=%CD%

REM
REM Unpack portable tools
REM
echo ******************************************************
echo.
echo Unpacking portable tools...
echo.
echo Note: Just click "Next" or "Finish" on any dialogs.
echo.
pushd buildtools\portable
PortableApps.comLauncher_2.2.1.paf.exe
PortableApps.comInstaller_3.4.4.paf.exe
pause
popd

REM
REM Copy together all files for building portable app
REM
pushd bin\Release

rmdir /S /Q winLAMEPortable 2> nul

mkdir winLAMEPortable

xcopy /S %ROOT%\source\portable\*.* winLAMEPortable\

mkdir winLAMEPortable\App\winLAME
copy %ROOT%\bin\Release\*.dll winLAMEPortable\App\winLAME\
copy %ROOT%\bin\Release\*.exe winLAMEPortable\App\winLAME\
copy %ROOT%\bin\Release\*.chm winLAMEPortable\App\winLAME\
copy %ROOT%\readme.txt winLAMEPortable\App\winLAME\Readme.txt
copy %ROOT%\Copying winLAMEPortable\App\winLAME\Copying.txt
copy "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\14.16.27012\vcredist_x86.exe" winLAMEPortable\App\winLAME\vcredist_x86_vc141.exe

mkdir winLAMEPortable\Data
copy %ROOT%\source\presets.xml  winLAMEPortable\App\winLAME\

REM
REM Building portable app
REM
echo ******************************************************
echo.
echo Building portable app...
echo.
cd winLAMEPortable
..\..\..\buildtools\portable\PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe %CD%
..\..\..\buildtools\portable\PortableApps.comInstaller\PortableApps.comInstaller.exe %CD%
cd ..
move winLAME*.paf.exe ..

popd

REM
REM Done
REM
echo ******************************************************
echo.
echo Portable exe is ready, in the folder:
echo "{winLAME-folder}\bin"
echo.

set ROOT=
