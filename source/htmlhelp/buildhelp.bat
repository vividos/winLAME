..\..\buildtools\hhkproc html/*.html html/howto/*.html html/misc/*.html html/pages/*.html html/techref/*.html
..\..\buildtools\hhc winlamehelp.hhp
copy ..\..\output\release\winlame.chm ..\..\output\debug\winlame.chm > nul
