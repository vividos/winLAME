//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2018 Michael Fink
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file LangCountryMapper.cpp
/// \brief mapping from language code to country code
//

#include "stdafx.h"
#include "LangCountryMapper.hpp"

LangCountryMapper::LangCountryMapper()
{
   m_mapCountryCodeToIndex[_T("ad")] = 0;
   m_mapCountryCodeToIndex[_T("ae")] = 1;
   m_mapCountryCodeToIndex[_T("af")] = 2;
   m_mapCountryCodeToIndex[_T("ag")] = 3;
   m_mapCountryCodeToIndex[_T("ai")] = 4;
   m_mapCountryCodeToIndex[_T("al")] = 5;
   m_mapCountryCodeToIndex[_T("am")] = 6;
   m_mapCountryCodeToIndex[_T("an")] = 7;
   m_mapCountryCodeToIndex[_T("ao")] = 8;
   m_mapCountryCodeToIndex[_T("ar")] = 9;
   m_mapCountryCodeToIndex[_T("as")] = 10;
   m_mapCountryCodeToIndex[_T("at")] = 11;
   m_mapCountryCodeToIndex[_T("au")] = 12;
   m_mapCountryCodeToIndex[_T("aw")] = 13;
   m_mapCountryCodeToIndex[_T("ax")] = 14;
   m_mapCountryCodeToIndex[_T("az")] = 15;
   m_mapCountryCodeToIndex[_T("ba")] = 16;
   m_mapCountryCodeToIndex[_T("bb")] = 17;
   m_mapCountryCodeToIndex[_T("bd")] = 18;
   m_mapCountryCodeToIndex[_T("be")] = 19;
   m_mapCountryCodeToIndex[_T("bf")] = 20;
   m_mapCountryCodeToIndex[_T("bg")] = 21;
   m_mapCountryCodeToIndex[_T("bh")] = 22;
   m_mapCountryCodeToIndex[_T("bi")] = 23;
   m_mapCountryCodeToIndex[_T("bj")] = 24;
   m_mapCountryCodeToIndex[_T("bm")] = 25;
   m_mapCountryCodeToIndex[_T("bn")] = 26;
   m_mapCountryCodeToIndex[_T("bo")] = 27;
   m_mapCountryCodeToIndex[_T("br")] = 28;
   m_mapCountryCodeToIndex[_T("bs")] = 29;
   m_mapCountryCodeToIndex[_T("bt")] = 30;
   m_mapCountryCodeToIndex[_T("bv")] = 31;
   m_mapCountryCodeToIndex[_T("bw")] = 32;
   m_mapCountryCodeToIndex[_T("by")] = 33;
   m_mapCountryCodeToIndex[_T("bz")] = 34;
   m_mapCountryCodeToIndex[_T("ca")] = 35;
   m_mapCountryCodeToIndex[_T("catalonia")] = 36;
   m_mapCountryCodeToIndex[_T("cc")] = 37;
   m_mapCountryCodeToIndex[_T("cd")] = 38;
   m_mapCountryCodeToIndex[_T("cf")] = 39;
   m_mapCountryCodeToIndex[_T("cg")] = 40;
   m_mapCountryCodeToIndex[_T("ch")] = 41;
   m_mapCountryCodeToIndex[_T("ci")] = 42;
   m_mapCountryCodeToIndex[_T("ck")] = 43;
   m_mapCountryCodeToIndex[_T("cl")] = 44;
   m_mapCountryCodeToIndex[_T("cm")] = 45;
   m_mapCountryCodeToIndex[_T("cn")] = 46;
   m_mapCountryCodeToIndex[_T("co")] = 47;
   m_mapCountryCodeToIndex[_T("cr")] = 48;
   m_mapCountryCodeToIndex[_T("cs")] = 49;
   m_mapCountryCodeToIndex[_T("cu")] = 50;
   m_mapCountryCodeToIndex[_T("cv")] = 51;
   m_mapCountryCodeToIndex[_T("cx")] = 52;
   m_mapCountryCodeToIndex[_T("cy")] = 53;
   m_mapCountryCodeToIndex[_T("cz")] = 54;
   m_mapCountryCodeToIndex[_T("de")] = 55;
   m_mapCountryCodeToIndex[_T("dj")] = 56;
   m_mapCountryCodeToIndex[_T("dk")] = 57;
   m_mapCountryCodeToIndex[_T("dm")] = 58;
   m_mapCountryCodeToIndex[_T("do")] = 59;
   m_mapCountryCodeToIndex[_T("dz")] = 60;
   m_mapCountryCodeToIndex[_T("ec")] = 61;
   m_mapCountryCodeToIndex[_T("ee")] = 62;
   m_mapCountryCodeToIndex[_T("eg")] = 63;
   m_mapCountryCodeToIndex[_T("eh")] = 64;
   m_mapCountryCodeToIndex[_T("england")] = 65;
   m_mapCountryCodeToIndex[_T("er")] = 66;
   m_mapCountryCodeToIndex[_T("es")] = 67;
   m_mapCountryCodeToIndex[_T("et")] = 68;
   m_mapCountryCodeToIndex[_T("europeanunion")] = 69;
   m_mapCountryCodeToIndex[_T("fam")] = 70;
   m_mapCountryCodeToIndex[_T("fi")] = 71;
   m_mapCountryCodeToIndex[_T("fj")] = 72;
   m_mapCountryCodeToIndex[_T("fk")] = 73;
   m_mapCountryCodeToIndex[_T("fm")] = 74;
   m_mapCountryCodeToIndex[_T("fo")] = 75;
   m_mapCountryCodeToIndex[_T("fr")] = 76;
   m_mapCountryCodeToIndex[_T("ga")] = 77;
   m_mapCountryCodeToIndex[_T("gb")] = 78;
   m_mapCountryCodeToIndex[_T("gd")] = 79;
   m_mapCountryCodeToIndex[_T("ge")] = 80;
   m_mapCountryCodeToIndex[_T("gf")] = 81;
   m_mapCountryCodeToIndex[_T("gh")] = 82;
   m_mapCountryCodeToIndex[_T("gi")] = 83;
   m_mapCountryCodeToIndex[_T("gl")] = 84;
   m_mapCountryCodeToIndex[_T("gm")] = 85;
   m_mapCountryCodeToIndex[_T("gn")] = 86;
   m_mapCountryCodeToIndex[_T("gp")] = 87;
   m_mapCountryCodeToIndex[_T("gq")] = 88;
   m_mapCountryCodeToIndex[_T("gr")] = 89;
   m_mapCountryCodeToIndex[_T("gs")] = 90;
   m_mapCountryCodeToIndex[_T("gt")] = 91;
   m_mapCountryCodeToIndex[_T("gu")] = 92;
   m_mapCountryCodeToIndex[_T("gw")] = 93;
   m_mapCountryCodeToIndex[_T("gy")] = 94;
   m_mapCountryCodeToIndex[_T("hk")] = 95;
   m_mapCountryCodeToIndex[_T("hm")] = 96;
   m_mapCountryCodeToIndex[_T("hn")] = 97;
   m_mapCountryCodeToIndex[_T("hr")] = 98;
   m_mapCountryCodeToIndex[_T("ht")] = 99;
   m_mapCountryCodeToIndex[_T("hu")] = 100;
   m_mapCountryCodeToIndex[_T("id")] = 101;
   m_mapCountryCodeToIndex[_T("ie")] = 102;
   m_mapCountryCodeToIndex[_T("il")] = 103;
   m_mapCountryCodeToIndex[_T("in")] = 104;
   m_mapCountryCodeToIndex[_T("io")] = 105;
   m_mapCountryCodeToIndex[_T("iq")] = 106;
   m_mapCountryCodeToIndex[_T("ir")] = 107;
   m_mapCountryCodeToIndex[_T("is")] = 108;
   m_mapCountryCodeToIndex[_T("it")] = 109;
   m_mapCountryCodeToIndex[_T("jm")] = 110;
   m_mapCountryCodeToIndex[_T("jo")] = 111;
   m_mapCountryCodeToIndex[_T("jp")] = 112;
   m_mapCountryCodeToIndex[_T("ke")] = 113;
   m_mapCountryCodeToIndex[_T("kg")] = 114;
   m_mapCountryCodeToIndex[_T("kh")] = 115;
   m_mapCountryCodeToIndex[_T("ki")] = 116;
   m_mapCountryCodeToIndex[_T("km")] = 117;
   m_mapCountryCodeToIndex[_T("kn")] = 118;
   m_mapCountryCodeToIndex[_T("kp")] = 119;
   m_mapCountryCodeToIndex[_T("kr")] = 120;
   m_mapCountryCodeToIndex[_T("kw")] = 121;
   m_mapCountryCodeToIndex[_T("ky")] = 122;
   m_mapCountryCodeToIndex[_T("kz")] = 123;
   m_mapCountryCodeToIndex[_T("la")] = 124;
   m_mapCountryCodeToIndex[_T("lb")] = 125;
   m_mapCountryCodeToIndex[_T("lc")] = 126;
   m_mapCountryCodeToIndex[_T("li")] = 127;
   m_mapCountryCodeToIndex[_T("lk")] = 128;
   m_mapCountryCodeToIndex[_T("lr")] = 129;
   m_mapCountryCodeToIndex[_T("ls")] = 130;
   m_mapCountryCodeToIndex[_T("lt")] = 131;
   m_mapCountryCodeToIndex[_T("lu")] = 132;
   m_mapCountryCodeToIndex[_T("lv")] = 133;
   m_mapCountryCodeToIndex[_T("ly")] = 134;
   m_mapCountryCodeToIndex[_T("ma")] = 135;
   m_mapCountryCodeToIndex[_T("mc")] = 136;
   m_mapCountryCodeToIndex[_T("md")] = 137;
   m_mapCountryCodeToIndex[_T("me")] = 138;
   m_mapCountryCodeToIndex[_T("mg")] = 139;
   m_mapCountryCodeToIndex[_T("mh")] = 140;
   m_mapCountryCodeToIndex[_T("mk")] = 141;
   m_mapCountryCodeToIndex[_T("ml")] = 142;
   m_mapCountryCodeToIndex[_T("mm")] = 143;
   m_mapCountryCodeToIndex[_T("mn")] = 144;
   m_mapCountryCodeToIndex[_T("mo")] = 145;
   m_mapCountryCodeToIndex[_T("mp")] = 146;
   m_mapCountryCodeToIndex[_T("mq")] = 147;
   m_mapCountryCodeToIndex[_T("mr")] = 148;
   m_mapCountryCodeToIndex[_T("ms")] = 149;
   m_mapCountryCodeToIndex[_T("mt")] = 150;
   m_mapCountryCodeToIndex[_T("mu")] = 151;
   m_mapCountryCodeToIndex[_T("mv")] = 152;
   m_mapCountryCodeToIndex[_T("mw")] = 153;
   m_mapCountryCodeToIndex[_T("mx")] = 154;
   m_mapCountryCodeToIndex[_T("my")] = 155;
   m_mapCountryCodeToIndex[_T("mz")] = 156;
   m_mapCountryCodeToIndex[_T("na")] = 157;
   m_mapCountryCodeToIndex[_T("nc")] = 158;
   m_mapCountryCodeToIndex[_T("ne")] = 159;
   m_mapCountryCodeToIndex[_T("nf")] = 160;
   m_mapCountryCodeToIndex[_T("ng")] = 161;
   m_mapCountryCodeToIndex[_T("ni")] = 162;
   m_mapCountryCodeToIndex[_T("nl")] = 163;
   m_mapCountryCodeToIndex[_T("no")] = 164;
   m_mapCountryCodeToIndex[_T("np")] = 165;
   m_mapCountryCodeToIndex[_T("nr")] = 166;
   m_mapCountryCodeToIndex[_T("nu")] = 167;
   m_mapCountryCodeToIndex[_T("nz")] = 168;
   m_mapCountryCodeToIndex[_T("om")] = 169;
   m_mapCountryCodeToIndex[_T("pa")] = 170;
   m_mapCountryCodeToIndex[_T("pe")] = 171;
   m_mapCountryCodeToIndex[_T("pf")] = 172;
   m_mapCountryCodeToIndex[_T("pg")] = 173;
   m_mapCountryCodeToIndex[_T("ph")] = 174;
   m_mapCountryCodeToIndex[_T("pk")] = 175;
   m_mapCountryCodeToIndex[_T("pl")] = 176;
   m_mapCountryCodeToIndex[_T("pm")] = 177;
   m_mapCountryCodeToIndex[_T("pn")] = 178;
   m_mapCountryCodeToIndex[_T("pr")] = 179;
   m_mapCountryCodeToIndex[_T("ps")] = 180;
   m_mapCountryCodeToIndex[_T("pt")] = 181;
   m_mapCountryCodeToIndex[_T("pw")] = 182;
   m_mapCountryCodeToIndex[_T("py")] = 183;
   m_mapCountryCodeToIndex[_T("qa")] = 184;
   m_mapCountryCodeToIndex[_T("re")] = 185;
   m_mapCountryCodeToIndex[_T("ro")] = 186;
   m_mapCountryCodeToIndex[_T("rs")] = 187;
   m_mapCountryCodeToIndex[_T("ru")] = 188;
   m_mapCountryCodeToIndex[_T("rw")] = 189;
   m_mapCountryCodeToIndex[_T("sa")] = 190;
   m_mapCountryCodeToIndex[_T("sb")] = 191;
   m_mapCountryCodeToIndex[_T("sc")] = 192;
   m_mapCountryCodeToIndex[_T("scotland")] = 193;
   m_mapCountryCodeToIndex[_T("sd")] = 194;
   m_mapCountryCodeToIndex[_T("se")] = 195;
   m_mapCountryCodeToIndex[_T("sg")] = 196;
   m_mapCountryCodeToIndex[_T("sh")] = 197;
   m_mapCountryCodeToIndex[_T("si")] = 198;
   m_mapCountryCodeToIndex[_T("sj")] = 199;
   m_mapCountryCodeToIndex[_T("sk")] = 200;
   m_mapCountryCodeToIndex[_T("sl")] = 201;
   m_mapCountryCodeToIndex[_T("sm")] = 202;
   m_mapCountryCodeToIndex[_T("sn")] = 203;
   m_mapCountryCodeToIndex[_T("so")] = 204;
   m_mapCountryCodeToIndex[_T("sr")] = 205;
   m_mapCountryCodeToIndex[_T("st")] = 206;
   m_mapCountryCodeToIndex[_T("sv")] = 207;
   m_mapCountryCodeToIndex[_T("sy")] = 208;
   m_mapCountryCodeToIndex[_T("sz")] = 209;
   m_mapCountryCodeToIndex[_T("tc")] = 210;
   m_mapCountryCodeToIndex[_T("td")] = 211;
   m_mapCountryCodeToIndex[_T("tf")] = 212;
   m_mapCountryCodeToIndex[_T("tg")] = 213;
   m_mapCountryCodeToIndex[_T("th")] = 214;
   m_mapCountryCodeToIndex[_T("tj")] = 215;
   m_mapCountryCodeToIndex[_T("tk")] = 216;
   m_mapCountryCodeToIndex[_T("tl")] = 217;
   m_mapCountryCodeToIndex[_T("tm")] = 218;
   m_mapCountryCodeToIndex[_T("tn")] = 219;
   m_mapCountryCodeToIndex[_T("to")] = 220;
   m_mapCountryCodeToIndex[_T("tr")] = 221;
   m_mapCountryCodeToIndex[_T("tt")] = 222;
   m_mapCountryCodeToIndex[_T("tv")] = 223;
   m_mapCountryCodeToIndex[_T("tw")] = 224;
   m_mapCountryCodeToIndex[_T("tz")] = 225;
   m_mapCountryCodeToIndex[_T("ua")] = 226;
   m_mapCountryCodeToIndex[_T("ug")] = 227;
   m_mapCountryCodeToIndex[_T("um")] = 228;
   m_mapCountryCodeToIndex[_T("us")] = 229;
   m_mapCountryCodeToIndex[_T("uy")] = 230;
   m_mapCountryCodeToIndex[_T("uz")] = 231;
   m_mapCountryCodeToIndex[_T("va")] = 232;
   m_mapCountryCodeToIndex[_T("vc")] = 233;
   m_mapCountryCodeToIndex[_T("ve")] = 234;
   m_mapCountryCodeToIndex[_T("vg")] = 235;
   m_mapCountryCodeToIndex[_T("vi")] = 236;
   m_mapCountryCodeToIndex[_T("vn")] = 237;
   m_mapCountryCodeToIndex[_T("vu")] = 238;
   m_mapCountryCodeToIndex[_T("wales")] = 239;
   m_mapCountryCodeToIndex[_T("wf")] = 240;
   m_mapCountryCodeToIndex[_T("ws")] = 241;
   m_mapCountryCodeToIndex[_T("ye")] = 242;
   m_mapCountryCodeToIndex[_T("yt")] = 243;
   m_mapCountryCodeToIndex[_T("za")] = 244;
   m_mapCountryCodeToIndex[_T("zm")] = 245;
   m_mapCountryCodeToIndex[_T("zw")] = 246;
}

LPCTSTR LangCountryMapper::CountryCodeFromLanguageCode(UINT uiLanguageCode) const
{
   // map country codes, according to http://msdn.microsoft.com/en-us/library/0h88fahh(VS.85).aspx
   switch (uiLanguageCode)
   {
   case 0x0436: return _T("af"); // Afrikaans
   case 0x041C: return _T("sq"); // Albanian
   case 0x3801: return _T("ar-ae"); // Arabic - United Arab Emirates
   case 0x3C01: return _T("ar-bh"); // Arabic - Bahrain
   case 0x1401: return _T("ar-dz"); // Arabic - Algeria
   case 0x0C01: return _T("ar-eg"); // Arabic - Egypt
   case 0x0801: return _T("ar-iq"); // Arabic - Iraq
   case 0x2C01: return _T("ar-jo"); // Arabic - Jordan
   case 0x3401: return _T("ar-kw"); // Arabic - Kuwait
   case 0x3001: return _T("ar-lb"); // Arabic - Lebanon
   case 0x1001: return _T("ar-ly"); // Arabic - Libya
   case 0x1801: return _T("ar-ma"); // Arabic - Morocco
   case 0x2001: return _T("ar-om"); // Arabic - Oman
   case 0x4001: return _T("ar-qa"); // Arabic - Qatar
   case 0x0401: return _T("ar-sa"); // Arabic - Saudi Arabia
   case 0x2801: return _T("ar-sy"); // Arabic - Syria
   case 0x1C01: return _T("ar-tn"); // Arabic - Tunisia
   case 0x2401: return _T("ar-ye"); // Arabic - Yemen
   case 0x042B: return _T("hy"); // Armenian
   case 0x042C: return _T("az-az"); // Azeri - Latin
   case 0x082C: return _T("az-az"); // Azeri - Cyrillic
   case 0x042D: return _T("eu"); // Basque
   case 0x0423: return _T("be"); // Belarusian
   case 0x0402: return _T("bg"); // Bulgarian
   case 0x0403: return _T("ca"); // Catalan
   case 0x0804: return _T("zh-cn"); // Chinese - China
   case 0x0C04: return _T("zh-hk"); // Chinese - Hong Kong SAR
   case 0x1404: return _T("zh-mo"); // Chinese - Macau SAR
   case 0x1004: return _T("zh-sg"); // Chinese - Singapore
   case 0x0404: return _T("zh-tw"); // Chinese - Taiwan
   case 0x041A: return _T("hr"); // Croatian
   case 0x0405: return _T("cs"); // Czech
   case 0x0406: return _T("da"); // Danish
   case 0x0413: return _T("nl-nl"); // Dutch - Netherlands
   case 0x0813: return _T("nl-be"); // Dutch - Belgium
   case 0x0C09: return _T("en-au"); // English - Australia
   case 0x2809: return _T("en-bz"); // English - Belize
   case 0x1009: return _T("en-ca"); // English - Canada
   case 0x2409: return _T("en-cb"); // English - Caribbean
   case 0x1809: return _T("en-ie"); // English - Ireland
   case 0x2009: return _T("en-jm"); // English - Jamaica
   case 0x1409: return _T("en-nz"); // English - New Zealand
   case 0x3409: return _T("en-ph"); // English - Phillippines
   case 0x1C09: return _T("en-za"); // English - Southern Africa
   case 0x2C09: return _T("en-tt"); // English - Trinidad
   case 0x0809: return _T("en-gb"); // English - Great Britain
   case 0x0409: return _T("en-us"); // English - United States
   case 0x0425: return _T("et"); // Estonian
   case 0x0429: return _T("fa"); // Farsi
   case 0x040B: return _T("fi"); // Finnish
   case 0x0438: return _T("fo"); // Faroese
   case 0x040C: return _T("fr-fr"); // French - France
   case 0x080C: return _T("fr-be"); // French - Belgium
   case 0x0C0C: return _T("fr-ca"); // French - Canada
   case 0x140C: return _T("fr-lu"); // French - Luxembourg
   case 0x100C: return _T("fr-ch"); // French - Switzerland
   case 0x083C: return _T("gd-ie"); // Gaelic - Ireland
   case 0x043C: return _T("gd"); // Gaelic - Scotland
   case 0x0407: return _T("de-de"); // German - Germany
   case 0x0C07: return _T("de-at"); // German - Austria
   case 0x1407: return _T("de-li"); // German - Liechtenstein
   case 0x1007: return _T("de-lu"); // German - Luxembourg
   case 0x0807: return _T("de-ch"); // German - Switzerland
   case 0x0408: return _T("el"); // Greek
   case 0x040D: return _T("he"); // Hebrew
   case 0x0439: return _T("hi"); // Hindi
   case 0x040E: return _T("hu"); // Hungarian
   case 0x040F: return _T("is"); // Icelandic
   case 0x0421: return _T("id"); // Indonesian
   case 0x0410: return _T("it-it"); // Italian - Italy
   case 0x0810: return _T("it-ch"); // Italian - Switzerland
   case 0x0411: return _T("ja"); // Japanese
   case 0x0412: return _T("ko"); // Korean
   case 0x0426: return _T("lv"); // Latvian
   case 0x0427: return _T("lt"); // Lithuanian
   case 0x042F: return _T("mk"); // F.Y.R.O. Macedonia
   case 0x043E: return _T("ms-my"); // Malay - Malaysia
   case 0x083E: return _T("ms-bn"); // Malay – Brunei
   case 0x043A: return _T("mt"); // Maltese
   case 0x044E: return _T("mr"); // Marathi
   case 0x0414: return _T("no-no"); // Norwegian - Bokml
   case 0x0814: return _T("no-no"); // Norwegian - Nynorsk
   case 0x0415: return _T("pl"); // Polish
   case 0x0816: return _T("pt-pt"); // Portuguese - Portugal
   case 0x0416: return _T("pt-br"); // Portuguese - Brazil
   case 0x0417: return _T("rm"); // Raeto-Romance
   case 0x0418: return _T("ro"); // Romanian - Romania
   case 0x0818: return _T("ro-mo"); // Romanian - Republic of Moldova
   case 0x0419: return _T("ru"); // Russian
   case 0x0819: return _T("ru-mo"); // Russian - Republic of Moldova
   case 0x044F: return _T("sa"); // Sanskrit
   case 0x0C1A: return _T("sr-sp"); // Serbian - Cyrillic
   case 0x081A: return _T("sr-sp"); // Serbian - Latin
   case 0x0432: return _T("tn"); // Setsuana
   case 0x0424: return _T("sl"); // Slovenian
   case 0x041B: return _T("sk"); // Slovak
   case 0x042E: return _T("sb"); // Sorbian
   case 0x040A: return _T("es-es"); // Spanish - Spain (Traditional)
   case 0x2C0A: return _T("es-ar"); // Spanish - Argentina
   case 0x400A: return _T("es-bo"); // Spanish - Bolivia
   case 0x340A: return _T("es-cl"); // Spanish - Chile
   case 0x240A: return _T("es-co"); // Spanish - Colombia
   case 0x140A: return _T("es-cr"); // Spanish - Costa Rica
   case 0x1C0A: return _T("es-do"); // Spanish - Dominican Republic
   case 0x300A: return _T("es-ec"); // Spanish - Ecuador
   case 0x100A: return _T("es-gt"); // Spanish - Guatemala
   case 0x480A: return _T("es-hn"); // Spanish - Honduras
   case 0x080A: return _T("es-mx"); // Spanish - Mexico
   case 0x4C0A: return _T("es-ni"); // Spanish - Nicaragua
   case 0x180A: return _T("es-pa"); // Spanish - Panama
   case 0x280A: return _T("es-pe"); // Spanish - Peru
   case 0x500A: return _T("es-pr"); // Spanish - Puerto Rico
   case 0x3C0A: return _T("es-py"); // Spanish - Paraguay
   case 0x440A: return _T("es-sv"); // Spanish - El Salvador
   case 0x380A: return _T("es-uy"); // Spanish - Uruguay
   case 0x200A: return _T("es-ve"); // Spanish - Venezuela
   case 0x0430: return _T("st"); // Southern Sotho
   case 0x0441: return _T("sw"); // Swahili
   case 0x041D: return _T("sv-se"); // Swedish - Sweden
   case 0x081D: return _T("sv-fi"); // Swedish - Finland
   case 0x0449: return _T("ta"); // Tamil
   case 0x0444: return _T("tt"); // Tatar
   case 0x041E: return _T("th"); // Thai
   case 0x041F: return _T("tr"); // Turkish
   case 0x0431: return _T("ts"); // Tsonga
   case 0x0422: return _T("uk"); // Ukrainian
   case 0x0420: return _T("ur"); // Urdu
   case 0x0843: return _T("uz-uz"); // Uzbek - Cyrillic
   case 0x0443: return _T("uz-uz"); // Uzbek – Latin
   case 0x042A: return _T("vi"); // Vietnamese
   case 0x0434: return _T("xh"); // Xhosa
   case 0x043D: return _T("yi"); // Yiddish
   case 0x0435: return _T("zu"); // Zulu
   default:
      ATLASSERT(false);
      break;
   }

   return NULL;
}

int LangCountryMapper::IndexFromLanguageCode(UINT uiLanguageCode) const
{
   // get country code from language code
   LPCTSTR pszCountryCode = CountryCodeFromLanguageCode(uiLanguageCode);
   if (pszCountryCode == NULL || *pszCountryCode == 0)
      return -1;

   // special meaning
   CString cszCountryCode(pszCountryCode);
   if (cszCountryCode == _T("en-us"))
      cszCountryCode = _T("us");

   // remove sub code
   int iPos = cszCountryCode.Find(_T('-'));
   if (iPos != -1)
      cszCountryCode = cszCountryCode.Left(iPos);

   // get flag index from country code
   std::map<CString, int>::const_iterator iter = m_mapCountryCodeToIndex.find(cszCountryCode);
   if (iter == m_mapCountryCodeToIndex.end())
      return -1;

   return iter->second;
}
