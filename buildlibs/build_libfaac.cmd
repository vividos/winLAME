@echo on
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2019 Michael Fink
REM
REM Downloads libfaac from github.com/knik0 and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=faac-1_30

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

REM download package
set URL=https://github.com/knik0/faac/archive/1_30.zip

if not exist %PREFIX%.zip powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.zip}"

REM unzip
rmdir /s /q %PREFIX%\ 2> nul
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.zip

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\project\msvc

REM update Toolset to v142, in order to use VS2019
powershell -Command "& {(Get-Content libfaac_dll.vcxproj) -replace \"v141\",\"v142\" | out-file libfaac_dll.vcxproj}"
REM also update framework
powershell -Command "& {(Get-Content libfaac_dll.vcxproj) -replace \"8.1\",\"10.0\" | out-file libfaac_dll.vcxproj}"

REM compile
msbuild libfaac_dll.vcxproj /m /property:Configuration=Release,Platform=Win32

popd

REM copy artifacts
copy "%PREFIX%\project\msvc\bin\Release\libfaac_dll.dll" ..\source\libraries\
copy "%PREFIX%\project\msvc\bin\Release\libfaac_dll.lib" ..\source\libraries\lib\libfaac_dll.lib

pause
