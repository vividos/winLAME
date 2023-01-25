@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (C) 2000-2023 Michael Fink
REM
REM Runs winLAME Unit Tests and collects coverage information
REM

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles%\Microsoft Visual Studio\2022\Community

REM
REM Preparations
REM
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

REM
REM Extract OpenCppCoverage build tools
REM
pushd ..\buildtools\OpenCppCoverage
"%ProgramFiles%\7-Zip\7z.exe" x -y -oOpenCppCoverage OpenCppCoverage-x64-0.9.8.0.zip
copy SonarQube.dll OpenCppCoverage\plugins\exporter
PATH=%PATH%;%CD%\OpenCppCoverage
popd

REM
REM Build Debug|x86
REM
msbuild winlame\unittest\unittest.vcxproj /m /property:Configuration=Debug /property:Platform=x86 /target:Build

REM
REM Run unit tests
REM
OpenCppCoverage.exe ^
   --continue_after_cpp_exception --cover_children ^
   --sources winlame\encoder ^
   --export_type SonarQube:winlame-coverage-SonarQube.xml ^
   --export_type html:CoverageReport ^
   --modules unittest.dll ^
   -- "%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe" ^
   "..\bin\Debug\unittest.dll" /Platform:x86 /InIsolation /logger:trx

pause
