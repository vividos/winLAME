@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2016 Michael Fink
REM
REM Runs doxygen to document sourcecode
REM

doxygen doxygen.cfg
..\..\buildtools\hhc ..\..\bin\doxygen\html\index.hhp
exit 0
