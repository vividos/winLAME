REM
REM BuildPortable.cmd - Builds winLAME Portable
REM

set ROOT=%CD%

cd bin\Release

rmdir /S /Q winLAMEPortable 2> nul

mkdir winLAMEPortable
REM copy AppNamePortable.exe
REM copy help.html

mkdir winLAMEPortable\App

mkdir winLAMEPortable\App\winLAME
copy %ROOT%\bin\Release\*.dll winLAMEPortable\App\winLAME\
copy %ROOT%\bin\Release\*.exe winLAMEPortable\App\winLAME\

mkdir winLAMEPortable\App\AppInfo
copy %ROOT%\source\portable\appinfo.ini winLAMEPortable\App\AppInfo\
copy %ROOT%\source\winlame\res\winlame.ico winLAMEPortable\App\AppInfo\appicon.ico
copy %ROOT%\source\portable\appicon*.png winLAMEPortable\App\AppInfo\

mkdir winLAMEPortable\App\AppInfo\Launcher
copy %ROOT%\source\portable\Splash.jpg winLAMEPortable\App\AppInfo\Launcher\
copy %ROOT%\source\portable\winLAMEPortable.ini  winLAMEPortable\App\AppInfo\Launcher\

mkdir winLAMEPortable\Data
copy %ROOT%\source\presets.xml  winLAMEPortable\Data\

mkdir winLAMEPortable\Other

mkdir winLAMEPortable\Other\Source
copy %ROOT%\source\portable\AppNamePortable.ini winLAMEPortable\Other\Source\
copy %ROOT%\source\portable\LauncherLicense.txt winLAMEPortable\Other\Source\
copy %ROOT%\source\portable\other_source_readme.txt winLAMEPortable\Other\Source\Readme.txt
REM EULA.txt
REM copy %ROOT%\docs\Copying.GPL 

cd ..\..
