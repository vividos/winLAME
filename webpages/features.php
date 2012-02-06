<?php
$title = "winLAME features";
include "header.php";
?>

<h2>winLAME feature list</h2>

<p>
winLAME has the following features:
</p>

<ul>
 <li>Encoding and decoding of many audio formats, including:
  <ul>
   <li>.mp3 via <a href="http://lame.sourceforge.net/">LAME mp3 encoder</a> (encoding) and <a href="http://www.underbit.com/products/mad/">MAD</a> (decoding)</li>
   <li>.ogg <a href="http://www.vorbis.com/">Ogg Vorbis</a></li>
   <li>.aac via <a href="http://www.audiocoding.com/index.html">libfaac/libfaad</a></li>
   <li>.wav, .aiff, .au, .voc and many more, via <a href="http://www.mega-nerd.com/libsndfile/">libsndfile</a></li>
   <li>.wma via Windows Media Audio codec</li>
   <li>.flac via <a href="http://flac.sourceforge.net/">FLAC</a> library</li>
  </ul>
 </li>
 <li>Uses LAME features, including:
  <ul>
   <li>high quality and optimized mp3 encoding</li>
   <li>nogap encoding of continuous-mix-cd's</li>
   <li>optimized 3DNow! and SSE routines</li>
   <li>ID3 v1 and v2 tagging</li>
  </ul>
 </li>
 <li>Easy-to-use wizard-like user interface style for easy encoding setup</li>
 <li>CD Audio extraction (aka. CD ripping), including <a href="http://www.freedb.org/">freedb</a> support</li>
 <li>Presets for fast settings setup</li>
 <li>User interface translations to english and german language</li>
 <li>Batch Processing</li>
 <li>Detailed HTML Help File</li>
 <li>Easy install- and uninstall process</li>
 <li>Small size</li>
</ul>

<h2>Licensing</h2>

<p>
winLAME is distributed and licensed under the GNU General Public License (GPL). That means, the source code is freely available. See the <a href="http://www.gnu.org/copyleft/gpl.html">GNU General Public License</a> page for more information. Most libraries used by winLAME are distributed under the <a href="http://www.gnu.org/copyleft/lesser.html">GNU Lesser GPL</a>.
</p>

<p>
Note that personal and/or commercial use of compiled versions of the LAME encoding enging (including the DLL distributed with winLAME) <i>may</i> require a patent license in some countries.
</p>


<?php include "footer.php"; ?>
