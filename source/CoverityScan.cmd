@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2017 Michael Fink
REM
REM runs Coverity analysis build
REM

rmdir .\cov-int /s /q 2> nul
del cov-int.zip 2> nul

REM set up build environment
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

set PATH=%PATH%;D:\devel\tools\cov-analysis-win64-2017.07\bin

REM restore NuGet packages
..\buildtools\nuget restore ..\winlame.sln

REM build solution
cov-build --dir cov-int ^
   msbuild ..\winlame.sln /property:Configuration=Release /property:Platform=Win32 /target:Rebuild

"%ProgramFiles%\7-Zip\7z.exe" a cov-int.zip .\cov-int

pause
