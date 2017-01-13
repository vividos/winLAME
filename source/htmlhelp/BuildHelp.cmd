@echo off
REM
REM winLAME - a frontend for the LAME encoding engine
REM Copyright (c) 2000-2017 Michael Fink
REM
REM Builds HTML help
REM

..\..\buildtools\hhkproc html/*.html html/howto/*.html html/misc/*.html html/pages/*.html html/techref/*.html
..\..\buildtools\hhc winLAMEhelp.hhp

copy ..\..\intermediate\htmlhelp\winLAME.chm %1\winLAME.chm 2> nul
exit 0
