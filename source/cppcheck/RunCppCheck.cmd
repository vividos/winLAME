REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (C) 2016-2026 Michael Fink
REM
REM Runs CppCheck to check sourcecode
REM

set PATH=%PATH%;"C:\Program Files\Cppcheck\"

REM when started via command line, assume "no xml" and "current folder"
set INTDIR=%1
if "%INTDIR%" == "" set INTDIR=%CD%\

REM when xml is passed as second param, set format and output file
set FORMAT=
set OUTFILE=%INTDIR%cppcheck.txt
if "%2" == "xml" set FORMAT=--xml
if "%2" == "xml" set OUTFILE=%INTDIR%cppcheck-Results.xml

mkdir ..\..\intermediate\cppcheck_build 2> nul

REM run cppcheck
REM -I <dir>            Include path
REM -i <dir>            Ignore path
REM --suppressions-list=<file>   File with suppressed warnings
REM -j 4                Multithreading
REM --platform=win32W   Platform specific types
REM --language=c++      Language (file extensions)
REM --std=c++20         Language (syntax)
REM --enable=all        Enable warnings
REM --template=vs       Output format for warnings
REM --check-config
cppcheck.exe ^
   ..\winlame ^
   -I "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\atlmfc\include" ^
   -I ..\..\intermediate\vcpkg_installed\x86-windows\include\ ^
   -I ..\winlame\ ^
   -I ..\winlame\classic\ ^
   -I ..\winlame\ui\ ^
   -i ..\winlame\unittest\ ^
   -i ..\libraries\include\ ^
   --cppcheck-build-dir=..\..\intermediate\cppcheck_build ^
   --language=c++ ^
   --std=c++20 ^
   --library=windows.cfg ^
   --library=microsoft_atl.cfg ^
   --library=microsoft_sal.cfg ^
   --library=microsoft_unittest.cfg ^
   --library=wtl.cfg ^
   -DWIN32 -D_WINDOWS -DNDEBUG -D_UNICODE -D__cplusplus ^
   -D_MSC_VER=1951 ^
   --suppressions-list=cppcheck-suppress.txt ^
   %FORMAT% ^
   --enable=all ^
   -j 4 ^
   --template=vs 2> %OUTFILE%
