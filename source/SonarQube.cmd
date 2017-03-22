@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2017 Michael Fink
REM
REM runs SonarQube analysis build
REM

call "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\Common7\Tools\vsvars32.bat"

REM
REM Extract SonarQube build tools
REM
cd ..\buildtools\SonarQube

"%ProgramFiles%\7-Zip\7z.exe" x -y build-wrapper-win-x86.zip
"%ProgramFiles%\7-Zip\7z.exe" x -y -osonar-scanner-msbuild sonar-scanner-msbuild-2.2.0.24.zip
cd ..\..\source

PATH=%PATH%;%CD%\..\buildtools\SonarQube\build-wrapper-win-x86;%CD%\..\buildtools\SonarQube\sonar-scanner-msbuild

REM
REM Build using SonarQube scanner for MSBuild
REM
cd ..
rmdir .\bw-output /s /q 2> nul

SonarQube.Scanner.MSBuild.exe begin ^
    /k:"winLAME" ^
    /v:"1.0" ^
    /d:"sonar.cfamily.build-wrapper-output=%CD%\bw-output" ^
    /d:"sonar.host.url=https://sonarqube.com" ^
    /d:"sonar.organization=vividos-github" ^
    /d:"sonar.login=3a12fee6a7d1e60cfbf6a8daa3554cfc6f2bfa29"

build-wrapper-win-x86-64.exe --out-dir bw-output msbuild winlame.sln /property:Configuration=Release /property:Platform=Win32 /target:Rebuild

SonarQube.Scanner.MSBuild.exe end /d:"sonar.login=3a12fee6a7d1e60cfbf6a8daa3554cfc6f2bfa29"

pause
