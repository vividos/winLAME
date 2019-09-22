# Developer Readme for winLAME

## Compiling winLAME

In order to compile winLAME, you just need Microsoft Visual Studio 2019. Any
dition will do, including Community, which I use to develop winLAME.

Open the winlame.sln in Visual Studio, set the startup project to "winLAME",
the configuration to "Debug" and hit F5 to compile and run winLAME.

For some of the projects, tools are necessary to be installed, e.g. doxygen,
cppcheck, the WiX Visual Studio addin or the HTML Help Workshop.

All output (object files, intermediate files and so on) will be put into sub
folders of the main folder named "bin", "lib" and "intermediate".

## Repository structure

The following list describes the folders in the winLAME source code
repository:

- buildtools

  Contains buildtools needed for building winLAME. among them are the HTML
  help compiler, SonarQube tools and the PortableApps installer.

- bin, lib, intermediate

  These folders don't exist in the repository, but will be created in your
  working copy when you compile the project.

- source

  Contains the source files used by winLAME. Contains all project files
  (*.vcxproj) and various scripts to create a build.

Let's take a deeper look at this folder:

- source\htmlhelp

  Contains files for the HTML help file, the project, table of contents and
  index file. Contains the subfolder "html" in which all html and png's reside.

- source\libraries

  Contains include header and lib files for the third-party libraries.

- source\winlame

  Contains the files for the winLAME UI. Has a folder "res" which contains
  the binary resource files used, a folder "encoder" containing the encoder
  backend and the folder "preset" for the preset management.

- source\nlame

   Contains code of the nlame API that wraps the normal LAME API.

## Translation Guide

This chapter describes how to translate winLAME into other languages. Since
winLAME 2009 it is possible to provide the winLAME author with a translation
for your native language. Note that you must provide translation under the
Creative Commons Attribution Share Alike license. See here for more:
   http://creativecommons.org/about/licenses/

There are two ways to translate the dialog and string resources from winLAME,
depending on if you are a developer or not. The easy way is using Resource
Hacker, and the more complicated way involves creating a translated resoure
DLL.

### Translation with Resource Hacker

You can use the tool Resource Hacker to translate all texts, by opening the
winLAME.exe or the german translation DLL winLAME.0407.dll and start
translating all resources that are listed in the tool. Please don't change
dialog layouts or control positions and sizes if possible.

You can send me the translated binary file so I can incorporate the texts in
a new Visual Studio resource project.

### Visual Studio resource project

Translations use another project file in the source\winlame folder, with the
file names looking like: `winlame_resXXXX.vcxproj`´. The XXXX stands for the
country code of the language. You can find the appropriate country code in
the file LangCountryMapper.cpp, in the method CountryCodeFromLanguageCode(),
starting at line 282.

Copy the german project file winlame_res0407.vcxproj and all other resource
files with 0704 in the name and replace them with your language code. Also
replace the file names in the .vcxproj file.

After adding the newly created project file to the winlame.sln solution, you
can use the Visual Studio resource view to translate any elements. Compile the
project to produce the winLAME.XXXX.dll file in the "bin" folder. It is
automatically recognized by winLAME and is shown in the settings dialog.

After translating you can send me the project files in order to incorporate
them in the project.

## Use Cases for winLAME

### Use Case: Encode audio files to different formats

Steps:

Expected result:

### Use Case: Read CD tracks and encode them to audio files

Steps:

Expected result:

## Test Cases for winLAME

### Encoding files

Case: Use filename with characters in a local codepage (like umlauts)
Expected result: Encoded files should contain correct filenames.

Case: Try to convert invalid or non-audio files
Expected result: An error message that the file is invalid or no input module
could be found should be shown.

Case:
Expected result:

Case:
Expected result:

### Encoding CDs

Case: Try to encode audio CD that is scratched
Epected result:

Case: Try to encode Data CD
Expected result: Error is reported in 

Case: Insert CD after "Encoding CD" is selected
Expected result: After some seconds, the tracks need to be detected and
listed.

Case: Try to request FreeDB or CoverArt data without internet connection
Expected result: After a while an error message should be reported.

Case: Use characters in a local codepage (like umlauts) in the CD or Track
title or other metadata
Expected result: Encoded files should contain correct metadata (check with
e.g. Mp3Tag).

### Classic Mode

Case: Try cancelling encoding from Encoding page
Expected result: First, a message asking if tasks should be stopped should
appear, and when answering with Yes, the Classic mode start page should
appear.

Case: Watch progress of encoding in classic mode in Task Bar icon
Expected result: The task bar should show a progress background while encoding
files.

Case: Switch to modern mode while encoding in Classic mode
Epected result: Encoding should continue, showing the task list.

## Release Checklist

Here's a checklist of what to do before each release:

- Update version number in the following files:
  * version.h
  * config.wxi
  * doxygen.cfg
  * source\portable\App\AppInfo\appinfo.ini
  * AppVeyor Settings
  * SonarQube build script
- Update copyright year in the following files:
  * about.html
  * about.????.html
  * setup\License.winLAME.rtf
  * htmlhelp index page
  * readme.txt
- Check all external libraries for updates and update them
- Check all NuGet packages and update them
- Check external tools for updates and update them (doxygen, cppcheck, wix)
- Build all projects in Release|Win32
- Run all unit tests
- Fix cppcheck errors and doxygen warnings
- Compile all project with /analyze and fix all warnings
- Use SonarQube to find even more warnings and fix them
- Test built .msi setup if it installs properly
- Do a short smoke test, encoding all test files
- Test portable zip archive by extracting and starting winLAME
- create winLAME portable with BuildPortable.cmd
- Fix all errors, check them in and push all changes
- Tag the git repository with tag "winlame-yyyy-beta-z" and push the tag
- Add a release and upload result .msi file, to releases on sourceforge.net
- Update website and upload it to sourceforge.net
- Done

## Library licenses
