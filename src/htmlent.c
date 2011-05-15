/*
  Copyright (C) 2006, 2011 Marius L. Jøhndal

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "htmlent.h"

GHashTable *htmlent_hash_new(void)
{
  GHashTable *h;

  h = g_hash_table_new(g_str_hash, g_str_equal);

  g_hash_table_insert(h, "nbsp", "&#160;");
  g_hash_table_insert(h, "iexcl", "&#161;");
  g_hash_table_insert(h, "cent", "&#162;");
  g_hash_table_insert(h, "pound", "&#163;");
  g_hash_table_insert(h, "curren", "&#164;");
  g_hash_table_insert(h, "yen", "&#165;");
  g_hash_table_insert(h, "brvbar", "&#166;");
  g_hash_table_insert(h, "sect", "&#167;");
  g_hash_table_insert(h, "uml", "&#168;");
  g_hash_table_insert(h, "copy", "&#169;");
  g_hash_table_insert(h, "ordf", "&#170;");
  g_hash_table_insert(h, "laquo", "&#171;");
  g_hash_table_insert(h, "not", "&#172;");
  g_hash_table_insert(h, "shy", "&#173;");
  g_hash_table_insert(h, "reg", "&#174;");
  g_hash_table_insert(h, "macr", "&#175;");
  g_hash_table_insert(h, "deg", "&#176;");
  g_hash_table_insert(h, "plusmn", "&#177;");
  g_hash_table_insert(h, "sup2", "&#178;");
  g_hash_table_insert(h, "sup3", "&#179;");
  g_hash_table_insert(h, "acute", "&#180;");
  g_hash_table_insert(h, "micro", "&#181;");
  g_hash_table_insert(h, "para", "&#182;");
  g_hash_table_insert(h, "middot", "&#183;");
  g_hash_table_insert(h, "cedil", "&#184;");
  g_hash_table_insert(h, "sup1", "&#185;");
  g_hash_table_insert(h, "ordm", "&#186;");
  g_hash_table_insert(h, "raquo", "&#187;");
  g_hash_table_insert(h, "frac14", "&#188;");
  g_hash_table_insert(h, "frac12", "&#189;");
  g_hash_table_insert(h, "frac34", "&#190;");
  g_hash_table_insert(h, "iquest", "&#191;");
  g_hash_table_insert(h, "Agrave", "&#192;");
  g_hash_table_insert(h, "Aacute", "&#193;");
  g_hash_table_insert(h, "Acirc", "&#194;");
  g_hash_table_insert(h, "Atilde", "&#195;");
  g_hash_table_insert(h, "Auml", "&#196;");
  g_hash_table_insert(h, "Aring", "&#197;");
  g_hash_table_insert(h, "AElig", "&#198;");
  g_hash_table_insert(h, "Ccedil", "&#199;");
  g_hash_table_insert(h, "Egrave", "&#200;");
  g_hash_table_insert(h, "Eacute", "&#201;");
  g_hash_table_insert(h, "Ecirc", "&#202;");
  g_hash_table_insert(h, "Euml", "&#203;");
  g_hash_table_insert(h, "Igrave", "&#204;");
  g_hash_table_insert(h, "Iacute", "&#205;");
  g_hash_table_insert(h, "Icirc", "&#206;");
  g_hash_table_insert(h, "Iuml", "&#207;");
  g_hash_table_insert(h, "ETH", "&#208;");
  g_hash_table_insert(h, "Ntilde", "&#209;");
  g_hash_table_insert(h, "Ograve", "&#210;");
  g_hash_table_insert(h, "Oacute", "&#211;");
  g_hash_table_insert(h, "Ocirc", "&#212;");
  g_hash_table_insert(h, "Otilde", "&#213;");
  g_hash_table_insert(h, "Ouml", "&#214;");
  g_hash_table_insert(h, "times", "&#215;");
  g_hash_table_insert(h, "Oslash", "&#216;");
  g_hash_table_insert(h, "Ugrave", "&#217;");
  g_hash_table_insert(h, "Uacute", "&#218;");
  g_hash_table_insert(h, "Ucirc", "&#219;");
  g_hash_table_insert(h, "Uuml", "&#220;");
  g_hash_table_insert(h, "Yacute", "&#221;");
  g_hash_table_insert(h, "THORN", "&#222;");
  g_hash_table_insert(h, "szlig", "&#223;");
  g_hash_table_insert(h, "agrave", "&#224;");
  g_hash_table_insert(h, "aacute", "&#225;");
  g_hash_table_insert(h, "acirc", "&#226;");
  g_hash_table_insert(h, "atilde", "&#227;");
  g_hash_table_insert(h, "auml", "&#228;");
  g_hash_table_insert(h, "aring", "&#229;");
  g_hash_table_insert(h, "aelig", "&#230;");
  g_hash_table_insert(h, "ccedil", "&#231;");
  g_hash_table_insert(h, "egrave", "&#232;");
  g_hash_table_insert(h, "eacute", "&#233;");
  g_hash_table_insert(h, "ecirc", "&#234;");
  g_hash_table_insert(h, "euml", "&#235;");
  g_hash_table_insert(h, "igrave", "&#236;");
  g_hash_table_insert(h, "iacute", "&#237;");
  g_hash_table_insert(h, "icirc", "&#238;");
  g_hash_table_insert(h, "iuml", "&#239;");
  g_hash_table_insert(h, "eth", "&#240;");
  g_hash_table_insert(h, "ntilde", "&#241;");
  g_hash_table_insert(h, "ograve", "&#242;");
  g_hash_table_insert(h, "oacute", "&#243;");
  g_hash_table_insert(h, "ocirc", "&#244;");
  g_hash_table_insert(h, "otilde", "&#245;");
  g_hash_table_insert(h, "ouml", "&#246;");
  g_hash_table_insert(h, "divide", "&#247;");
  g_hash_table_insert(h, "oslash", "&#248;");
  g_hash_table_insert(h, "ugrave", "&#249;");
  g_hash_table_insert(h, "uacute", "&#250;");
  g_hash_table_insert(h, "ucirc", "&#251;");
  g_hash_table_insert(h, "uuml", "&#252;");
  g_hash_table_insert(h, "yacute", "&#253;");
  g_hash_table_insert(h, "thorn", "&#254;");
  g_hash_table_insert(h, "yuml", "&#255;");
  g_hash_table_insert(h, "quot", "&#34;");
  g_hash_table_insert(h, "amp", "&#38;#38;");
  g_hash_table_insert(h, "lt", "&#38;#60;");
  g_hash_table_insert(h, "gt", "&#62;");
  g_hash_table_insert(h, "apos  ", "&#39;");
  g_hash_table_insert(h, "OElig", "&#338;");
  g_hash_table_insert(h, "oelig", "&#339;");
  g_hash_table_insert(h, "Scaron", "&#352;");
  g_hash_table_insert(h, "scaron", "&#353;");
  g_hash_table_insert(h, "Yuml", "&#376;");
  g_hash_table_insert(h, "circ", "&#710;");
  g_hash_table_insert(h, "tilde", "&#732;");
  g_hash_table_insert(h, "ensp", "&#8194;");
  g_hash_table_insert(h, "emsp", "&#8195;");
  g_hash_table_insert(h, "thinsp", "&#8201;");
  g_hash_table_insert(h, "zwnj", "&#8204;");
  g_hash_table_insert(h, "zwj", "&#8205;");
  g_hash_table_insert(h, "lrm", "&#8206;");
  g_hash_table_insert(h, "rlm", "&#8207;");
  g_hash_table_insert(h, "ndash", "&#8211;");
  g_hash_table_insert(h, "mdash", "&#8212;");
  g_hash_table_insert(h, "lsquo", "&#8216;");
  g_hash_table_insert(h, "rsquo", "&#8217;");
  g_hash_table_insert(h, "sbquo", "&#8218;");
  g_hash_table_insert(h, "ldquo", "&#8220;");
  g_hash_table_insert(h, "rdquo", "&#8221;");
  g_hash_table_insert(h, "bdquo", "&#8222;");
  g_hash_table_insert(h, "dagger", "&#8224;");
  g_hash_table_insert(h, "Dagger", "&#8225;");
  g_hash_table_insert(h, "permil", "&#8240;");
  g_hash_table_insert(h, "lsaquo", "&#8249;");
  g_hash_table_insert(h, "rsaquo", "&#8250;");
  g_hash_table_insert(h, "euro", "&#8364;");
  g_hash_table_insert(h, "fnof", "&#402;");
  g_hash_table_insert(h, "Alpha", "&#913;");
  g_hash_table_insert(h, "Beta", "&#914;");
  g_hash_table_insert(h, "Gamma", "&#915;");
  g_hash_table_insert(h, "Delta", "&#916;");
  g_hash_table_insert(h, "Epsilon", "&#917;");
  g_hash_table_insert(h, "Zeta", "&#918;");
  g_hash_table_insert(h, "Eta", "&#919;");
  g_hash_table_insert(h, "Theta", "&#920;");
  g_hash_table_insert(h, "Iota", "&#921;");
  g_hash_table_insert(h, "Kappa", "&#922;");
  g_hash_table_insert(h, "Lambda", "&#923;");
  g_hash_table_insert(h, "Mu", "&#924;");
  g_hash_table_insert(h, "Nu", "&#925;");
  g_hash_table_insert(h, "Xi", "&#926;");
  g_hash_table_insert(h, "Omicron", "&#927;");
  g_hash_table_insert(h, "Pi", "&#928;");
  g_hash_table_insert(h, "Rho", "&#929;");
  g_hash_table_insert(h, "Sigma", "&#931;");
  g_hash_table_insert(h, "Tau", "&#932;");
  g_hash_table_insert(h, "Upsilon", "&#933;");
  g_hash_table_insert(h, "Phi", "&#934;");
  g_hash_table_insert(h, "Chi", "&#935;");
  g_hash_table_insert(h, "Psi", "&#936;");
  g_hash_table_insert(h, "Omega", "&#937;");
  g_hash_table_insert(h, "alpha", "&#945;");
  g_hash_table_insert(h, "beta", "&#946;");
  g_hash_table_insert(h, "gamma", "&#947;");
  g_hash_table_insert(h, "delta", "&#948;");
  g_hash_table_insert(h, "epsilon", "&#949;");
  g_hash_table_insert(h, "zeta", "&#950;");
  g_hash_table_insert(h, "eta", "&#951;");
  g_hash_table_insert(h, "theta", "&#952;");
  g_hash_table_insert(h, "iota", "&#953;");
  g_hash_table_insert(h, "kappa", "&#954;");
  g_hash_table_insert(h, "lambda", "&#955;");
  g_hash_table_insert(h, "mu", "&#956;");
  g_hash_table_insert(h, "nu", "&#957;");
  g_hash_table_insert(h, "xi", "&#958;");
  g_hash_table_insert(h, "omicron", "&#959;");
  g_hash_table_insert(h, "pi", "&#960;");
  g_hash_table_insert(h, "rho", "&#961;");
  g_hash_table_insert(h, "sigmaf", "&#962;");
  g_hash_table_insert(h, "sigma", "&#963;");
  g_hash_table_insert(h, "tau", "&#964;");
  g_hash_table_insert(h, "upsilon", "&#965;");
  g_hash_table_insert(h, "phi", "&#966;");
  g_hash_table_insert(h, "chi", "&#967;");
  g_hash_table_insert(h, "psi", "&#968;");
  g_hash_table_insert(h, "omega", "&#969;");
  g_hash_table_insert(h, "thetasym", "&#977;");
  g_hash_table_insert(h, "upsih", "&#978;");
  g_hash_table_insert(h, "piv", "&#982;");
  g_hash_table_insert(h, "bull", "&#8226;");
  g_hash_table_insert(h, "hellip", "&#8230;");
  g_hash_table_insert(h, "prime", "&#8242;");
  g_hash_table_insert(h, "Prime", "&#8243;");
  g_hash_table_insert(h, "oline", "&#8254;");
  g_hash_table_insert(h, "frasl", "&#8260;");
  g_hash_table_insert(h, "weierp", "&#8472;");
  g_hash_table_insert(h, "image", "&#8465;");
  g_hash_table_insert(h, "real", "&#8476;");
  g_hash_table_insert(h, "trade", "&#8482;");
  g_hash_table_insert(h, "alefsym", "&#8501;");
  g_hash_table_insert(h, "larr", "&#8592;");
  g_hash_table_insert(h, "uarr", "&#8593;");
  g_hash_table_insert(h, "rarr", "&#8594;");
  g_hash_table_insert(h, "darr", "&#8595;");
  g_hash_table_insert(h, "harr", "&#8596;");
  g_hash_table_insert(h, "crarr", "&#8629;");
  g_hash_table_insert(h, "lArr", "&#8656;");
  g_hash_table_insert(h, "uArr", "&#8657;");
  g_hash_table_insert(h, "rArr", "&#8658;");
  g_hash_table_insert(h, "dArr", "&#8659;");
  g_hash_table_insert(h, "hArr", "&#8660;");
  g_hash_table_insert(h, "forall", "&#8704;");
  g_hash_table_insert(h, "part", "&#8706;");
  g_hash_table_insert(h, "exist", "&#8707;");
  g_hash_table_insert(h, "empty", "&#8709;");
  g_hash_table_insert(h, "nabla", "&#8711;");
  g_hash_table_insert(h, "isin", "&#8712;");
  g_hash_table_insert(h, "notin", "&#8713;");
  g_hash_table_insert(h, "ni", "&#8715;");
  g_hash_table_insert(h, "prod", "&#8719;");
  g_hash_table_insert(h, "sum", "&#8721;");
  g_hash_table_insert(h, "minus", "&#8722;");
  g_hash_table_insert(h, "lowast", "&#8727;");
  g_hash_table_insert(h, "radic", "&#8730;");
  g_hash_table_insert(h, "prop", "&#8733;");
  g_hash_table_insert(h, "infin", "&#8734;");
  g_hash_table_insert(h, "ang", "&#8736;");
  g_hash_table_insert(h, "and", "&#8743;");
  g_hash_table_insert(h, "or", "&#8744;");
  g_hash_table_insert(h, "cap", "&#8745;");
  g_hash_table_insert(h, "cup", "&#8746;");
  g_hash_table_insert(h, "int", "&#8747;");
  g_hash_table_insert(h, "there4", "&#8756;");
  g_hash_table_insert(h, "sim", "&#8764;");
  g_hash_table_insert(h, "cong", "&#8773;");
  g_hash_table_insert(h, "asymp", "&#8776;");
  g_hash_table_insert(h, "ne", "&#8800;");
  g_hash_table_insert(h, "equiv", "&#8801;");
  g_hash_table_insert(h, "le", "&#8804;");
  g_hash_table_insert(h, "ge", "&#8805;");
  g_hash_table_insert(h, "sub", "&#8834;");
  g_hash_table_insert(h, "sup", "&#8835;");
  g_hash_table_insert(h, "nsub", "&#8836;");
  g_hash_table_insert(h, "sube", "&#8838;");
  g_hash_table_insert(h, "supe", "&#8839;");
  g_hash_table_insert(h, "oplus", "&#8853;");
  g_hash_table_insert(h, "otimes", "&#8855;");
  g_hash_table_insert(h, "perp", "&#8869;");
  g_hash_table_insert(h, "sdot", "&#8901;");
  g_hash_table_insert(h, "lceil", "&#8968;");
  g_hash_table_insert(h, "rceil", "&#8969;");
  g_hash_table_insert(h, "lfloor", "&#8970;");
  g_hash_table_insert(h, "rfloor", "&#8971;");
  g_hash_table_insert(h, "lang", "&#9001;");
  g_hash_table_insert(h, "rang", "&#9002;");
  g_hash_table_insert(h, "loz", "&#9674;");
  g_hash_table_insert(h, "spades", "&#9824;");
  g_hash_table_insert(h, "clubs", "&#9827;");
  g_hash_table_insert(h, "hearts", "&#9829;");
  g_hash_table_insert(h, "diams", "&#9830;");

  return h;
}

void htmlent_hash_destroy(GHashTable *h)
{
  g_hash_table_destroy(h);
}
