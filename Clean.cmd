@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2020 Michael Fink
REM
REM Cleans the project folder from temporary files
REM

rmdir /s /q .vs 2> nul
rmdir /s /q intermediate 2> nul
rmdir /s /q lib 2> nul
rmdir /s /q bin 2> nul
rmdir /s /q packages 2> nul
rmdir /s /q buildtools\OpenCppCoverage\OpenCppCoverage 2> nul
rmdir /s /q buildtools\portable\PortableApps.comInstaller 2> nul
rmdir /s /q buildtools\portable\PortableApps.comLauncher 2> nul
rmdir /s /q buildtools\SonarQube\build-wrapper-win-x86 2> nul
rmdir /s /q buildtools\SonarQube\sonar-scanner-msbuild 2> nul
rmdir /s /q source\CoverageReport 2> nul
rmdir /s /q source\nlame\intermediate 2> nul
rmdir /s /q source\nlame\lib 2> nul
rmdir /s /q source\TestResults 2> nul
rmdir /s /q uwpapp\obj 2> nul
rmdir /s /q uwpapp\bin 2> nul
rmdir /s /q uwpapp\winlame 2> nul

pause
