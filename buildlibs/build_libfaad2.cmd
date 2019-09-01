@echo on
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2019 Michael Fink
REM
REM Downloads libfaad2 from github.com/knik0 and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=faad2-master

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

REM download package
set URL=https://codeload.github.com/knik0/faad2/zip/master

if not exist %PREFIX%.zip powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.zip}"

REM unzip
rmdir /s /q %PREFIX%\
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.zip

REM copy additional files
xcopy /s /y libfaad2-msvc\*.* %PREFIX%\

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\project\msvc

REM compile
msbuild libfaad2_dll.vcxproj /m /property:Configuration=Release,Platform=Win32

popd

REM copy artifacts
copy "%PREFIX%\project\msvc\bin\Release\libfaad2_dll\libfaad2.dll" ..\source\libraries\
copy "%PREFIX%\project\msvc\bin\Release\libfaad2_dll\libfaad2_dll.lib" ..\source\libraries\lib\libfaad2.lib

pause
