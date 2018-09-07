@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (C) 2000-2018 Michael Fink
REM
REM Runs winLAME Unit Tests and collects coverage information
REM

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community

REM and this to your OpenCppCoverage folder
set OPENCPPCOVERAGE=D:\devel\tools\OpenCppCoverage\

REM
REM Preparations
REM
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

set PATH=%PATH%;%OPENCPPCOVERAGE%

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
   --export_type cobertura:winLAME-coverage.xml ^
   --export_type html:CoverageReport ^
   --modules unittest.dll ^
   -- "%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe" ^
   "..\bin\Debug\unittest.dll" /Platform:x86 /InIsolation /logger:trx

REM   --excluded_sources packages\boost ^

pause
