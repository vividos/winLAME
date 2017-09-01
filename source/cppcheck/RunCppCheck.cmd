REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (C) 2016 Michael Fink
REM
REM runs CppCheck to check sourcecode
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

REM run cppcheck
REM -I <dir>            Include path
REM -i <dir>            Ignore path
REM --suppressions-list=<file>   File with suppressed warnings
REM -j 4                Multithreading
REM --platform=win32W   Platform specific types
REM --language=c++      Language (file extensions)
REM --std=c++11         Language (syntax)%
REM --enable=all        Enable warnings
REM --template vs       Output format for warnings
REM --check-config
cppcheck.exe ..\winlame ^
   -DWIN32 -D_WINDOWS -DNDEBUG -D_UNICODE -D__cplusplus -D_MSC_VER=1900 ^
   -D__ATLAPP_H__ -DNTDDI_WIN7=0x06010000 -DNTDDI_VERSION=0x06010000 ^
   -i ..\winlame\unittest\ ^
   --suppressions-list=cppcheck-suppress.txt ^
   --platform=win32W --language=c++ --std=c++11 %FORMAT% --enable=all ^
   -j 4 --template vs 2> %OUTFILE%
