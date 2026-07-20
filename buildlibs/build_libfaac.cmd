@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2026 Michael Fink
REM
REM Downloads libfaac from github.com/knik0 and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=faac-2.0

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles%\Microsoft Visual Studio\18\Community

REM download package
set URL=https://github.com/knik0/faac/archive/refs/tags/%PREFIX%.zip

if not exist %PREFIX%.zip (
	powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.zip}"
)

REM unzip
rmdir /s /q faac-%PREFIX%\ 2> nul
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.zip

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

REM copy project files
xcopy /S faac\*.* faac-%PREFIX%\

pushd faac-%PREFIX%\

echo #define PACKAGE "%PREFIX%" > include\config.h
echo #define PACKAGE_VERSION "%PREFIX%" >> include\config.h
echo #define MAX_CHANNELS 8 >> include\config.h
echo #define FAAC_SBR_DECIMATION 1 >> include\config.h
echo #define FAACAPI __declspec(dllexport) >> include\config.h

REM compile
msbuild libfaac_dll.vcxproj /m /property:Configuration=Release,Platform=Win32
msbuild libfaac_dll.vcxproj /m /property:Configuration=Release,Platform=x64

popd

REM copy artifacts
copy "faac-%PREFIX%\include\faac.h" ..\source\libraries\include\
copy "faac-%PREFIX%\bin\Win32\Release\libfaac_dll.dll" ..\source\libraries\bin\Win32\
copy "faac-%PREFIX%\bin\Win32\Release\libfaac_dll.lib" ..\source\libraries\lib\
copy "faac-%PREFIX%\bin\x64\Release\libfaac_dll.dll" ..\source\libraries\bin\x64\
copy "faac-%PREFIX%\bin\x64\Release\libfaac_dll.lib" ..\source\libraries\lib\x64\

pause
