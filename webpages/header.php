<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
 <title>winLAME home page</title>
 <meta http-equiv="content-type" content="text/html; charset=ISO-8859-1" />
 <meta name="author" content="Michael Fink" />
 <meta name="keyword" content="winLAME" />
 <link rel="icon" href="favicon.ico" />
 <style type="text/css"><!--
  body,p,td,h1,h2,h3 { font-family:Verdana,Tahoma,sans-serif; color:white; }
  body,p,td { font-size:11pt; }
  a:link { color:#a0a0a0 }
  a:visited { color:#ff8000;  }
  a:focus, a:hover, a:active { color:#000099; }
 //--></style>
 <!-- Piwik -->
 <script type="text/javascript">
  var pkBaseURL = (("https:" == document.location.protocol) ? "https://apps.sourceforge.net/piwik/winlame/" : "http://apps.sourceforge.net/piwik/winlame/");
  document.write(unescape("%3Cscript src='" + pkBaseURL + "piwik.js' type='text/javascript'%3E%3C/script%3E"));
 </script><script type="text/javascript">
  piwik_action_name = '';
  piwik_idsite = 1;
  piwik_url = pkBaseURL + "piwik.php";
  piwik_log(piwik_action_name, piwik_idsite, piwik_url);
 </script>
 <object><noscript><p><img src="http://apps.sourceforge.net/piwik/winlame/piwik.php?idsite=1" alt="piwik"/></p></noscript></object>
<!-- End Piwik Tag -->
<?php
   if ($addHighslide === true)
   {
?>
 <script type="text/javascript" src="/highslide/highslide.packed.js"></script>
 <link rel="stylesheet" type="text/css" href="/highslide/highslide.css" />
 <script type="text/javascript">
    // instead of editing the highslide.js file
    hs.graphicsDir = '/highslide/graphics/';
 </script>
<?php   
   }
?>
</head>
<body style="color:white; background-color:black; padding:8px; margin:0px">

<div style="width:700px; min-width:700px; max-width:700px; margin-left:auto; margin-right:auto; padding:0px">
  <div style="background-color:#ff8000; padding:8px">
    <img src="note.png" align="left" width="56" height="56" style="margin-right:16px" alt="note"/>
    <h1 style="padding:0px; margin:0px; height:56px">
<?php

   if ($title == "") $title = "winLAME home page";
   echo $title;

    ?></h1>
  </div>
  <div style="background-color:#202020">
    <p style="padding:6px; margin:0px">
      <a href="index.php">Home</a> |
      <a href="download.php">Downloads</a> |
      <a href="features.php">Features</a> |
      <a href="screenshots.php">Screenshots</a> |
      <a href="faq.php">FAQ</a> |
      <a href="http://sourceforge.net/projects/winlame/">Development</a>
    </p>
  </div>
  <div style="background-color:#0040c0; padding:8px">
  <!-- content start -->
