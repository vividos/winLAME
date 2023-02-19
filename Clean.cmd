@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2023 Michael Fink
REM
REM Cleans the project folder from temporary files
REM

echo Cleaning all files...

rmdir /s /q .vs 2> nul
rmdir /s /q .bw-output 2> nul
rmdir /s /q .sonar-cache 2> nul
rmdir /s /q .sonarqube 2> nul
rmdir /s /q intermediate 2> nul
rmdir /s /q lib 2> nul
rmdir /s /q bin 2> nul
rmdir /s /q packages 2> nul
rmdir /s /q vcpkg_installed 2> nul
rmdir /s /q buildtools\OpenCppCoverage\OpenCppCoverage 2> nul
rmdir /s /q buildtools\portable\PortableApps.comInstaller 2> nul
rmdir /s /q buildtools\portable\PortableApps.comLauncher 2> nul
rmdir /s /q buildtools\SonarQube\build-wrapper-win-x86 2> nul
rmdir /s /q buildtools\SonarQube\sonar-scanner-msbuild 2> nul
rmdir /s /q source\CoverageReport 2> nul
rmdir /s /q source\TestResults 2> nul
rmdir /s /q uwpapp\obj 2> nul
rmdir /s /q uwpapp\bin 2> nul
rmdir /s /q uwpapp\winlame 2> nul

del /S /Q *.user 2> nul
del /S /Q *.aps 2> nul
del /S /Q *.bml 2> nul
del /Q source\winlame\res\MainFrameRibbon.0402.h 2> nul
del /Q source\winlame\res\MainFrameRibbon.0402.rc 2> nul
del /Q source\winlame\res\MainFrameRibbon.0407.h 2> nul
del /Q source\winlame\res\MainFrameRibbon.0407.rc 2> nul
del /Q source\winlame\res\MainFrameRibbon.h 2> nul
del /Q source\winlame\res\MainFrameRibbon.rc 2> nul
del /Q source\htmlhelp\index.hhk 2> nul
del /Q source\LastCoverageResults.log 2> nul
del /Q source\winlame-coverage.xml 2> nul

echo Done.

pause
