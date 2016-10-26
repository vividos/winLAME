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

mkdir winLAMEPortable\Data
copy %ROOT%\source\presets.xml  winLAMEPortable\Data\

mkdir winLAMEPortable\Other

mkdir winLAMEPortable\Other\Source
copy %ROOT%\docs\Copying.GPL winLAMEPortable\Other\Source\LauncherLicense.txt
REM EULA.txt

cd ..\..
