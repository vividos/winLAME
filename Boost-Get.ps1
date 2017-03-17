#
# winLAME - a frontend for the LAME encoding engine
# Copyright (c) 2000-2017 Michael Fink
#
# Boost-Get.ps1 - Downloads the Boost version and extracts it, for use in Appveyor winLAME build
#
Write-Host "Boost-Get.ps1 - Downloads the Boost version and extracts it"
Write-Host ""

Add-Type -assembly "System.IO.Compression.Filesystem"

New-Item c:\temp -type directory
New-Item c:\devel -type directory

Write-Host "Downloading Boost.."
 (New-Object net.webclient).DownloadFile("https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.zip/download?use_mirror=autoselect", "c:\temp\boost_1_63_0.zip")

Write-Host "Extracting archive.."
[io.compression.zipfile]::ExtractToDirectory("c:\temp\boost_1_63_0.zip", "c:\devel\packages\")

subst d: c:\
