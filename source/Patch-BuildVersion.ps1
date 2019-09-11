<#
	winLAME - a frontend for the LAME encoding engine
	Copyright (c) 2000-2019 Michael Fink

.SYNOPSIS
	Writes a version number to the files version.h and config.wxi
#>

param (
	[Parameter(Mandatory=$true)][string]$version = "1.0.0.0"
)

Write-Host "Patching build version $version..."

# split version number
$array = $version.Split(".")
$majorVersion = $array[0]
$minorVersion = $array[1]
$releaseNumber = $array[2]
$buildNumber = $array[3]
$buildYear = Get-Date -format yyyy

# modify version.h
$versionHeader = Get-Content version.h

$versionHeader = $versionHeader -replace "MAJOR_VERSION [0-9]+","MAJOR_VERSION $majorVersion"
$versionHeader = $versionHeader -replace "MINOR_VERSION [0-9]+","MINOR_VERSION $minorVersion"
$versionHeader = $versionHeader -replace "RELEASE_NUMBER [0-9]+","RELEASE_NUMBER $releaseNumber"
$versionHeader = $versionHeader -replace "BUILD_NUMBER [0-9]+","BUILD_NUMBER $buildNumber"
$versionHeader = $versionHeader -replace "BUILD_YEAR [0-9]+","BUILD_YEAR $buildYear"

Out-File -FilePath version.h -InputObject $versionHeader -Encoding UTF8

# modify config.wxi
$configWxi = Get-Content setup\config.wxi

$configWxi = $configWxi -replace "MajorVersion = ""[0-9]+""","MajorVersion = ""$majorVersion"""
$configWxi = $configWxi -replace "MinorVersion = ""[0-9]+""","MinorVersion = ""$minorVersion"""
$configWxi = $configWxi -replace "ReleaseNumber = ""[0-9]+""","ReleaseNumber = ""$releaseNumber"""
$configWxi = $configWxi -replace "BuildNumber = ""[0-9]+""","BuildNumber = ""$buildNumber"""
$configWxi = $configWxi -replace "BuildYear = ""[0-9]+""","BuildYear = ""$buildYear"""

Out-File -FilePath  setup\config.wxi -InputObject $configWxi -Encoding UTF8

Write-Host "Done patching."
