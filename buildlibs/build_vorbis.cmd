@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2020 Michael Fink
REM
REM Downloads Vorbis and compiles it
REM

REM set this to the filename of the file to download
set PREFIX=libvorbis-1.3.7

REM set this to your Visual Studio installation folder
set VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community

REM download package
set URL=https://downloads.xiph.org/releases/vorbis/%PREFIX%.tar.gz

if not exist %PREFIX%.tar.gz powershell -Command "& {Invoke-WebRequest -Uri %URL% -Out %PREFIX%.tar.gz}"

REM unzip
rmdir /s /q %PREFIX%\
del %PREFIX%.tar
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar.gz
"c:\Program Files\7-Zip\7z.exe" x %PREFIX%.tar
del %PREFIX%.tar

REM copy additional files
xcopy /s /y libvorbis-msvc\*.* %PREFIX%\

REM also copy ogg headers
if not exist "include\ogg.h" echo "Error: Build libogg first before running this build script!"
if not exist "include\ogg.h" exit
mkdir "%PREFIX%\include\ogg" 2> nul
copy "include\ogg.h" "%PREFIX%\include\ogg\"
copy "include\os_types.h" "%PREFIX%\include\ogg\"
xcopy /s /y "libogg-1.3.4\win32\VS2015\Win32\ReleaseDLL\*.lib" "libogg-1.3.4\win32\VS2015\Win32\Release\"

REM set up Visual Studio
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"

pushd %PREFIX%\win32\VS2010

REM update .props file to use newer libogg
powershell -Command "& {(Get-Content libogg.props) -replace \"1.3.2\",\"1.3.4\" | out-file libogg.props}"

REM compile
msbuild libvorbis\libvorbis_dynamic.vcxproj /m /property:Configuration=Release,Platform=Win32

mkdir Win32\Release
copy libvorbis\Win32\Release Win32\Release

msbuild libvorbisfile\libvorbisfile_dynamic.vcxproj /m /property:Configuration=Release,Platform=Win32

popd

REM copy artifacts
copy %PREFIX%\win32\VS2010\libvorbis\Win32\Release\libvorbis.dll ..\source\libraries\
copy %PREFIX%\win32\VS2010\libvorbisfile\Win32\Release\libvorbisfile.dll ..\source\libraries\
copy %PREFIX%\win32\VS2010\libvorbis\Win32\Release\libvorbis.lib ..\source\libraries\lib\
copy %PREFIX%\win32\VS2010\libvorbisfile\Win32\Release\libvorbisfile.lib ..\source\libraries\lib\
copy %PREFIX%\include\vorbis\*.h ..\source\libraries\include\vorbis\

pause
