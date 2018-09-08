@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2017 Michael Fink
REM
REM Runs SonarCloud analysis build
REM

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community

REM
REM Preparations
REM
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

REM
REM Extract SonarQube build tools
REM
cd ..\buildtools\SonarQube

"%ProgramFiles%\7-Zip\7z.exe" x -y build-wrapper-win-x86.zip
"%ProgramFiles%\7-Zip\7z.exe" x -y -osonar-scanner-msbuild sonar-scanner-msbuild-3.0.2.656.zip
cd ..\..\source

PATH=%PATH%;%CD%\..\buildtools\SonarQube\build-wrapper-win-x86;%CD%\..\buildtools\SonarQube\sonar-scanner-msbuild

REM
REM Build using SonarQube scanner for MSBuild
REM
cd ..
rmdir .\bw-output /s /q 2> nul

msbuild winlame.sln /m /property:Configuration=Release,Platform=Win32 /target:Clean

SonarQube.Scanner.MSBuild.exe begin ^
    /k:"winLAME" ^
    /v:"2.17.4.0" ^
    /d:"sonar.cfamily.build-wrapper-output=%CD%\bw-output" ^
    /d:"sonar.host.url=https://sonarcloud.io" ^
    /d:"sonar.organization=vividos-github" ^
    /d:"sonar.login=%SONARLOGIN%"

REM
REM Rebuild Release|Win32
REM
build-wrapper-win-x86-64.exe --out-dir bw-output msbuild winlame.sln /property:Configuration=Release,Platform=Win32 /target:Build

SonarQube.Scanner.MSBuild.exe end /d:"sonar.login=%SONARLOGIN%"

pause
