<?php
$title = "";
include "header.php";
?>

<h2>About winLAME</h2>

<p>
winLAME is an encoder for several audio formats, including mp3 (MPEG Layer 3), Ogg Vorbis and more. winLAME lets you set up the encoding process with an easy-to-use wizard-style user interface.
</p>

<p>
winLAME supports many input audio formats, and uses the LAME mp3 encoding library for encoding and the MAD decoding library for decoding. See the <a href="features.php">features page</a> for a list of all supported formats.
</p>

<p>
winLAME uses the services provided by
<a href="http://sourceforge.net"><img src="http://sourceforge.net/sflogo.php?group_id=21193" width="88" height="31" border="0" align="top" alt="SourceForge Logo"/></a>
</p>

<h2>Latest news</h2>

<b>2011-xx-xx - winLAME 2011 release candidate 1 released</b><br/>

<p>
I released a new version of winLAME, mainly a bugfix release. The following bugs were fixed:
</p>

<ul>
  <li>Fixed decoding mono mp3 files that resulted in longer stretched sounding audio.</li>
  <li>Fixed bug with deleting source files and encoding to the same format (e.g. mp3 to mp3).</li>
  <li>Made all wizard dialogs resizable. Fixed a bug with maximizing the first page.</li>
  <li>Support for .m4a extension for AAC decoding (no support for Apple Lossless codec).</li>
</ul>

<p>
  Get the new installers in the <a href="download.php">download section</a>.
</p>

<hr width="80%"/>

Archived news items can be found in the <a href="archive.php">archive section</a>.

<?php include "footer.php"; ?>
