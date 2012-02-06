@echo off
if "%WIX_PATH%" == "" set WIX_PATH=D:\devel\tools\wix-v3\bin

title Building .msi file...
echo Building .msi file...
echo.

del winLAME.msi 2> nul
del winLAME.wixobj 2> nul
del UserInterface.wixobj 2> nul
del ErrorTextList.wixobj 2> nul

%WIX_PATH%\candle -nologo winLAME.wxs
%WIX_PATH%\candle -nologo UserInterface.wxs
%WIX_PATH%\candle -nologo ErrorTextList.wxs
%WIX_PATH%\light -nologo -sw1055 -sice:ICE03 -sice:ICE82 UserInterface.wixobj ErrorTextList.wixobj winLAME.wixobj -out winLAME.msi
REM ignore some errors based on http://blogs.msdn.com/astebner/archive/2007/02/13/building-an-msi-using-wix-v3-0-that-includes-the-vc-8-0-runtime-merge-modules.aspx

del ErrorTextList.wixobj
del UserInterface.wixobj
del winLAME.wixobj
