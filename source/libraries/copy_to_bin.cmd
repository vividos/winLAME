@echo off
mkdir ..\..\bin 2> nul
mkdir ..\..\bin\Debug 2> nul
mkdir ..\..\bin\Release 2> nul

copy bass.dll ..\..\bin\Debug
copy basscd.dll ..\..\bin\Debug
copy basswma.dll ..\..\bin\Debug
copy libFLAC.dll ..\..\bin\Debug
copy libmmd.dll ..\..\bin\Debug
copy libmp3lame.dll ..\..\bin\Debug
copy libsndfile-1.dll ..\..\bin\Debug
copy libvorbis.dll ..\..\bin\Debug
copy libspeex.dll ..\..\bin\Debug
copy libopus-0.dll ..\..\bin\Debug
copy libfaac.dll ..\..\bin\Debug
copy libfaad2.dll ..\..\bin\Debug
copy MrCrash.exe ..\..\bin\Debug

copy bass.dll ..\..\bin\Release
copy basscd.dll ..\..\bin\Release
copy basswma.dll ..\..\bin\Release
copy libFLAC.dll ..\..\bin\Release
copy libmmd.dll ..\..\bin\Release
copy libmp3lame.dll ..\..\bin\Release
copy libsndfile-1.dll ..\..\bin\Release
copy libvorbis.dll ..\..\bin\Release
copy libspeex.dll ..\..\bin\Release
copy libopus-0.dll ..\..\bin\Release
copy libfaac.dll ..\..\bin\Release
copy libfaad2.dll ..\..\bin\Release
copy MrCrash.exe ..\..\bin\Release

mkdir C:\ProgramData\winLAME 2> nul
copy ..\presets.xml C:\ProgramData\winLAME\presets.xml
