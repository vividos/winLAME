<?php
$title = "winLAME FAQ";
include "header.php";
?>

<h2>Frequent questions and their answers</h2>

<p>
<b>Q: In the latest winLAME version you can't choose the stereo mode
anymore and all mp3's are encoded as Joint Stereo. But I want to encode in
full stereo mode, as it would give me better quality. Can you add that option back into winLAME?</b>
<br/>
A: Starting from prerelease4 on, winLAME got a new user interface for the LAME options,
and I followed the LAME's teams recommendation (<a href="http://lame.sourceforge.net/lame_ui_example.html">see here</a>).
I asked one of the LAME team members if the stereo mode should be made
available in the UI again. Here's what Gabriel Bouvigne answered:
</p>

<blockquote>
I would strongly advise you to NOT provide this kind of choice to the 
end user. Activating legacy stereo instead of joint stereo will decrease 
quality in 99% of the cases.
Lame is internally able to switch between LR stereo and MS stereo on a 
frame basis, trying to use the most efficient mode on each frame. On 
usual signals, using LR instead of MS is quite inneficient, using more 
bits. When there are not enough available bits to stay at the 
transparency threshold, we must decrease quality. This situation is more 
likely to appear in legacy stereo mode.
</blockquote>

<blockquote>
I think that most users only want this because of placebo effect. They 
feel better if they can change default options, and because of this they 
are convinced (without blind tests, of course) that the results are better.
If we never offered the option, users would probably not mind, but now 
that we provided the choice in the past, they want it. Most AAC encoders 
(Apple, Nero) are not offering the choice, and are using MS or LR at 
their own convenience when the user is encoding a stereo track, and no 
one is complaining about it, mainly because those encoders never offered 
any other choice.
</blockquote>

<p>
<b>Q: How can I encode/decode AAC files?</b>
<br/>
A: For AAC encoding or decoding you need a version of libfaac.dll or libfaad2.dll in the winLAME folder. You can get an older version that may do from the <a href="http://www.rarewares.org/">Rarewares web site</a>, or you can go to <a href="http://www.audiocoding.com/">http://www.audiocoding.com/</a>, download the sources of the libraries and compile them for yourself. You can use Microsoft Visual C++ Express edition for this.
</p>

<p>
<b>Q: Can I copy a newer lame_enc.dll into winLAME's folder to update the LAME version?</b>
<br/>
A: No; winLAME uses a self-made API to LAME (which eases access for winLAME) that is packaged into a DLL called nLAME.dll. This DLL cannot be exchanged with other (renamed) DLLs that contain newer versions of LAME. When a new version is released, check back on this site if an update is available. The <a href="http://www.rarewares.org/">Rarewares web site</a> also often has a newer nLAME.dll that is compiled with the latest Intel ICL compiler.
</p>

<p>
<b>Q: Is there a possibility to encode multiple albums with winLAME?</b>
<br/>
A: Yes. Either you open winLAME multiple times, or you can use the <b>"use input file's folder as output location"</b> option on winLAME's Output Settings to write back the encoded files to their own folders.
</p>

<p>
<b>Q: Where can I file a bug report?</b>
<br/>
A: winLAME might have very rare bugs, but they usually don't prevent normal working with winLAME. winLAME does not crash or bluescreen your system (that might be a hint that you have a damaged driver somewhere). If you discovered a bug, describe and add it to the <a href="https://sourceforge.net/tracker/?group_id=21193&amp;atid=121193">winLAME bugtracker</a> on SourceForge or send it to <img src="http://uwadv.sourceforge.net/mailgray.png" alt="my mail address" />
</p>



<?php include "footer.php"; ?>