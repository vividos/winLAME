@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2020 Michael Fink
REM
REM Runs SonarCloud analysis build
REM

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

REM
REM Preparations
REM
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

if "%SONARLOGIN%" == "" echo "Environment variable SONARLOGIN is not set! Obtain a new token and set the environment variable!"
if "%SONARLOGIN%" == "" exit 1

REM
REM Extract SonarQube build tools
REM
pushd ..\buildtools\SonarQube
"%ProgramFiles%\7-Zip\7z.exe" x -y build-wrapper-win-x86.zip
"%ProgramFiles%\7-Zip\7z.exe" x -y -osonar-scanner-msbuild sonar-scanner-msbuild-5.0.4.24009-net46.zip
PATH=%PATH%;%CD%\build-wrapper-win-x86;%CD%\sonar-scanner-msbuild
popd

REM
REM Extract OpenCppCoverage build tools
REM
pushd ..\buildtools\OpenCppCoverage
"%ProgramFiles%\7-Zip\7z.exe" x -y -oOpenCppCoverage OpenCppCoverage-x64-0.9.8.0.zip
copy SonarQube.dll OpenCppCoverage\plugins\exporter
PATH=%PATH%;%CD%\OpenCppCoverage
popd

REM
REM Build using SonarQube scanner for MSBuild
REM
cd ..
rmdir .\.sonarqube /s /q 2> nul
rmdir .\bw-output /s /q 2> nul

msbuild winlame.sln /m /property:Configuration=SonarCloud,Platform=Win32 /target:Clean

SonarScanner.MSBuild.exe begin ^
    /k:"winLAME" ^
    /v:"2.20.0.0" ^
    /d:"sonar.cfamily.build-wrapper-output=%CD%\bw-output" ^
    /d:"sonar.coverageReportPaths=%CD%\source\winlame-coverage.xml" ^
    /d:"sonar.host.url=https://sonarcloud.io" ^
    /d:"sonar.cfamily.threads=4" ^
    /o:"vividos-github" ^
    /d:"sonar.login=%SONARLOGIN%" ^
    /d:sonar.cs.vstest.reportsPaths="%CD%\source\TestResults\*.trx"
if errorlevel 1 goto end

REM
REM Rebuild Release|Win32
REM
build-wrapper-win-x86-64.exe --out-dir bw-output msbuild winlame.sln /m /property:Configuration=SonarCloud,Platform=Win32 /target:Restore;Rebuild

REM
REM Run unit tests
REM
pushd source
OpenCppCoverage.exe ^
   --continue_after_cpp_exception --cover_children ^
   --sources winlame\encoder ^
   --export_type SonarQube:winlame-coverage.xml ^
   --export_type html:CoverageReport ^
   --modules unittest.dll ^
   -- "%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe" ^
   "..\bin\Release\unittest.dll" /Platform:x86 /InIsolation /logger:trx
popd

SonarScanner.MSBuild.exe end /d:"sonar.login=%SONARLOGIN%"

:end

pause
