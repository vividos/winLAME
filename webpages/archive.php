<?php
$title = "";
include "header.php";
?>

<h2>Archived winLAME news</h2>

<b>2010-06-28 - winLAME 2010 beta 2 released</b><br/>

<p>
A new version of winLAME was released, mainly a bugfix released. The following bugs were fixed:
</p>

<ul>
  <li>Fixed renaming output files from temporary file to final file.</li>
  <li>Fixed lockup when writing an output file is skipped, and the next file is about to be written.</li>
  <li>Fixed version check for libFLAC.dll; only version up to 1.1.2 are supported currently (porting
    winLAME to the API changes would be welcomed!)</li>
</ul>

<p>
  Get the new installers in the <a href="download.php">download section</a>.
</p>

<hr width="80%"/>

<b>2010-01-12 - winLAME 2010 beta 1 released</b><br/>

<p>
Since we're in 2010 now, I released winLAME with the new year name. The following changes were made:
</p>

<ul>
  <li>Fixed bug where encoding a file contained in a path containing non-ASCII characters would give an error.</li>
  <li>Upgraded LAME to 3.98.2, compiled with Intels ICL compiler</li>
  <li>Other small bug fixes</li>
</ul>

<p>
The files can be found in the <a href="download.php">download section</a> as usual.
</p>

<hr width="80%"/>

<b>2009-11-17 - Installer fixes for winLAME 2009 beta 2</b><br/>

<p>
After getting reports from some users about not being able to properly install winLAME 2009 beta 2, I re-released the installer today. The following bugs were fixed:
</p>

<ul>
  <li>After installing winLAME, the following error appeared: "This application has failed to start because the application configuration is incorrect."</li>
  <li>When trying to encode .wav files the error appeared: "Couldn't open input file (System error.)"</li>
  <li>When trying to encode to .wav format, the file could not be written, or the file ended up being created in the same folder as winLAME.exe, with the filename containing square symbol characters.</li>
</ul>

<p>
The files can be found in the <a href="download.php">download section</a> as usual. The broken installer package was removed.
</p>

<hr width="80%"/>

<b>2009-11-02 - winLAME 2009 beta 2 released</b><br/>

<p>
Today I released a new beta version of winLAME, with some bug fixes.
For those dying to get the latest version, head over to the <a href="download.php">download section</a>.
</p>

<p>
This release fixes some bugs that appeared in beta 1:
</p>

<ul>
  <li>The about page showed a blank page.</li>
  <li>When a playlist was written, the playlist file only contained the last encoded file.</li>
  <li>When converting to Wave format, the comboboxes showed no drop-down list.</li>
  <li>Fixed a crash when reading an mp3 file that contained invalid ID3v2 tags</li>
  <li>Fixed crash when a freedb query for a CD returned multiple entries</li>
  <li>Fixed transferring genre field to encoded files</li>
  <li>Fixed reading and writing tags for WMA.</li>
  <li>Fixed installer that didn't install correct msvcr80.dll and msvcp80.dll</li>
  <li>Fixed bug where track numbers was always set to the same value and genre field was always set to "Blues"</li>
  <li>Fixed bug where the ID3v2 tag always contained a large length value</li>
  <li>Added a patch from Adam Kropelin that provides reading FLAC metadata when converting from FLAC files.</li>
  <li>Updated BASS and libsndfile libraries to latest versions</li>
</ul>

<p>
Unfortunately no new translations were provided yet. If you'd like to translate winLAME to your
native language, you're welcome!
</p>

<hr width="80%"/>

<b>2009-05-13 - Translation Guide</b><br/>

<p>
I created a translation guide that describes how to translate winLAME into other languages. You can find it here: <a href="http://winlame.cvs.sourceforge.net/viewvc/winlame/winlame/docs/Translation-Guide.txt?view=markup">Translation Guide.txt</a>. If you have any questions about translating or if you want to know if translation to a certain language is already under way, please send me an email.
</p>

<hr width="80%"/>

<b>2009-04-16 - winLAME 2009 beta 1 released</b><br/>

<p>
In the last years I made some additions and changes to winLAME and fixed quite a few bugs. I decided to occasionally release a new version with updated libraries and bug fixes when I have time. Therefore winLAME got a new naming scheme. The year of release will be appended to the version name, and there will be a beta, a release candidate and a full release version for each version released.
</p>

<p>
<b>Note that this beta 1 release is not intended for production use!</b> It may contain bugs that may reduce audio quality and prevent some features to be used. Testing is encouraged, though. If you find a bug, describe and add it to the <a href="https://sourceforge.net/tracker/?group_id=21193&amp;atid=121193">winLAME bugtracker</a> on SourceForge or send it to <img src="http://uwadv.sourceforge.net/mailgray.png" alt="my mail address"/>.
</p>

<p>
Here's an (incomplete) list of changes:
</p>

<ul>
<li>Updated encoding/decoding libraries: LAME 3.98.2, libsndfile-1.0.19 (fixes security vulnerability, see <a href="http://secunia.com/advisories/33980/">Secunia advisory SA33980</a>), libvorbis 1.2.0</li>
<li>Support for reading/writing ID3v2 tags from/to .mp3 files</li>
<li>Translatable user interface (english and german, at the moment)</li>
<li>Full Unicode support for filenames</li>
<li>Numerous bug fixes (see the <a href="https://sourceforge.net/tracker/?group_id=21193&amp;atid=121193">winLAME bugtracker</a>)</li>
</ul>

<p>
Note: If you previously used custom presets using the presets.xml file, consult the help file for changes according to presets.
</p>

<p>
If you're interested in translating the user interface of winLAME, please send me an E-Mail. There's no money to be made, though, only eternal fame and glory :)
</p>

<hr width="80%"/>

<p>
<b>2006-02-09 - winLAME prerelease 4</b><br/>
This is a bugfix release. The following bugs are fixed:
<ul>
<li>crash when transcoding from mp3 to mp3</li>
<li>encoding was auto-started when going back and forth after all files are ripped from cd</li>
<li>fixed transferring track infos from CD audio to mp3 ID3v1 tags</li>
<li>on encode page, when there are no more files to encode, go directly to "input" page on back button</li>
</ul>
</p>

<p>
Be sure to uninstall all previous versions of winLAME and make sure all remaining files are removed before installing winLAME prerelease 4.
</p>

<hr width="80%"/>

<p>
<b>2005-12-13 - winLAME prerelease 3</b><br/>
Today I released prerelease 3 (go to the <a href="http://winlame.sourceforge.net/download.php">download section</a> directly), the next step to the final winLAME version. I tested this version in-depth so that it produces the same mp3 files as when using the command line lame.exe. While testing I found a rather serious bug that led to different files, so this version of winLAME is now the recommended one.
</p>

<p>
The new release for the first time is available as Windows Installer (.msi) file that helps to ease installation and uninstallation. Using the Windows Installer service on Windows is the most reliable way to install winLAME, even for LUA (Limited User Accounts) scenarios. I also updated other components of winLAME: It now includes an ICL compile of LAME 3.97 beta 1 (Thanks John Edwards!), libsndfile 1.0.12, libmad 0.15.1b and Ogg Vorbis 1.1.2 (from Rarewares).
</p>

<p>
The next release will contain only bug fixes to the program and an update to the help file. While using and testing winLAME please report all bugs that have to do with features in winLAME. Here's my email address: <img src="http://uwadv.sourceforge.net/mailgray.png" />.
</p>


<hr width="80%"/>

<p>
<b>2005-07-26 - winLAME prerelease 2</b><br/>
I did a new build, <a href="https://sourceforge.net/project/showfiles.php?group_id=21193&package_id=15384&release_id=344965">"prerelease 2"</a> with some new features. winLAME now supports ripping CDs via the <a href="http://www.un4seen.com/bass.html">BASS</a> CD library, as well as some enhancements and new formats done by DeXT. (for FLAC support, drop <a href="http://cvs.sourceforge.net/viewcvs.py/*checkout*/winlame/winlame/source/libraries/libFLAC.dll">libFLAC.dll</a> into winLAME's install folder). winLAME also has LAME 3.96.1, MAD 0.15.1 beta and libsndfile 1.0.11 on board. Note that this version wasn't tested well, so use at your own risk. Reporting bugs in the new features is welcome! I may find some minutes from time to time to work on winLAME.
</p>

<hr width="80%"/>

<p>
<b>2004-03-22 - new winLAME build</b><br/>
<a href="http://sourceforge.net/project/showfiles.php?group_id=21193">Released</a> a new build "pre1" (don't know of what it is a "pre", but oh well :-). It has the LAME-3.95.1, MAD 0.15.0b and libsndfile 1.0.5 on board. If you previously had problems with ID3v2 tags on your files that won't load into winLAME, try this. Note: this version is completely untested, so use at your own risk! And no, this project won't restart.
</p>

<hr width="80%"/>

<p>
<b>2003-06-11 - new winLAME DLLs by john33</b><br/>
john33 (from the <a href="http://www.hydrogenaudio.org/">Hyrdogenaudio site</a>) compiled new DLLs for use with winLAME. New ones were the MAD 0.15.0b and LAME 3.90.3 (using ICL4.5 and Dibrom's switches). Just head over to <a href="http://rarewares.hydrogenaudio.org/">Rarewares</a>, change to chapter "mp3" and search for winLAME. Many thanks john33!
</p>

<hr width="80%"/>

<p>
<b>2003-01-27 - home page redesign</b><br/>
Today the winLAME home page was redesigned to have a new and permanent design. I also added a FAQ about the most frequently asked questions about winLAME.
</p>

<hr width="80%"/>

<p>
<b>2002-12-11 - LAME-3.93.1 library available</b><br>
John33 compiled a LAME-3.93.1 version nLAME.dll for use in winLAME. It is available <a href="http://homepage.ntlworld.com/jfe1205/nLAME.zip">here</a>. The needed libmmd.dll is available <a href="http://homepage.ntlworld.com/jfe1205/">here</a>.
</p>

<hr width="80%">

<p>
<b>2002-11-22 - winLAME release candidate 3</b><br>
Just released winLAME rc3. The following things were added:<br>
- libsndfile 1.0.1 library<br>
- ogg vorbis 1.0 library<br>
- usage of Winamp 2.x input modules<br>
- completed documentation<br>
- added transcoding warning<br>
- added "input file path as output folder" feature<br>
- new logo (thanks davis!)<br>
and no, LAME-3.93 won't be included.
</p>

<p>
This will be the last release of winLAME, as I don't have time to work on it. Besides that I think there are superior formats like Ogg Vorbis that are much better than mp3. My initial goal was to create a frontend for the LAME encoder without those many confusing options, and I think I managed it. Time to move on to new things.
</p>

<p>
For all the novice users of audio compression software, head over to the <a href="http://www.audio-illumination.org/forums/">HydrogenAudio forums</a> and learn a about audio coding and the better formats. It's a bit exhausting to comb through the threads, but often very informative.
</p>

<hr width="80%">

<?php include "footer.php"; ?>
