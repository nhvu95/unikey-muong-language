﻿/*------------------------------------------------------------------------------
UniKey - Vietnamese Keyboard for Windows
Copyright (C) 1998-2002 Pham Kim Long
Contact: longp@cslab.felk.cvut.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--------------------------------------------------------------------------------*/
#include "prehdr.h"
#include "keyhook.h"
#include "encode.h"
#include "vnconv.h"

#include <stdio.h>
/*
Explanation of encoding in DT table:
In DT:
- Bits 0-4: index of vowels
- Bits 5-8: index of macro keys (e.g. TELEX: w, W, [, ]
- Bits 9-13: index of double characters (including: a, d, e, o, a^, dd, e^, o^, a(, u+...)
- Bits 14-17: index of tone marks
- Bits 18-21: determines the index of the tone associated with a character, e.g tone_index(a') = 1.
- Bit 22: is set if this character is used to type a breve. (in telex, they are w, W)
- Bit 24: soft separator. Soft separators don't cause the buffer to be cleared.
- Bit 25: separator flag. Buffer must be cleared when separator character is encountered.
- Bit 26-28: index of VNI double marks ('6', '7', '8', '9')
- Bit 29: Word stops
*/


/*/////////////////////////////////////////////////////////////////////

STEPS TO ADD NEW CHARSETS

1. For 1-byte charsets
- Append charset to the end of table MapBD, MapBK, MapBW, MapBT
- Insert charset information into CharsetTable (to any position you like), provide following information:
	+ Charset name
	+ The index of the charset in the table MapBD (the same as in MapBK, MapBW, MapBT)
- Increase TOTAL_CHARSET constant (in keycons.h) by 1 to reflex the number of supported charsets

2. For 2-byte charsets
- Append charset to the end of BOTH tables UniVnL (for lowercase), UniVnH (for upper case)
- Insert charset information into CharsetTable (to any position you like), provide following information:
	+ Charset name
	+ The index of the charset in the tables UniVnL (the same as in UniVnH) PLUS 0x101
- Increase TOTAL_CHARSET constant by 1 in keycons.h to reflex the number of supported charsets

//////////////////////////////////////////////////////////////////////*/
CharsetInfo CharsetTable[TOTAL_CHARSET] = {
	{_TEXT("Unicode"),						0x101,	CONV_CHARSET_UNICODE,	UNICODE_UCS2},
	{_TEXT("TCVN3 (ABC)"),					0,		CONV_CHARSET_TCVN3,		0},
	{_TEXT("VNI Windows"),					0x102,	CONV_CHARSET_VNIWIN,	0},
	{_TEXT("VIQR"),							0x100,	CONV_CHARSET_VIQR,		0},
	{_TEXT("Vietnamese locale CP 1258"),	0x109,	CONV_CHARSET_WINCP1258,	0},
	{_TEXT("UTF-8 Literal"),				0x101,	CONV_CHARSET_UNIUTF8,	UNICODE_UTF8},
	{_TEXT("NCR Decimal"),					0x101,	CONV_CHARSET_UNIREF,	UNICODE_REF},
	{_TEXT("NCR Hex"),						0x101,	CONV_CHARSET_UNIREF_HEX,UNICODE_HEX},
	{_TEXT("Composite Unicode (To hop)"),	0x108,	CONV_CHARSET_UNIDECOMPOSED,	0},
	{_TEXT("VISCII"),						0x106,	CONV_CHARSET_VISCII,	0},
	{_TEXT("VPS"),							0x104,	CONV_CHARSET_VPS,		0},
	{_TEXT("BK HCM 2"),						0x103,	CONV_CHARSET_BKHCM2,	0},
	{_TEXT("BK HCM 1"),						0x107,	CONV_CHARSET_BKHCM1,	0},
	{_TEXT("Vietware X"),					0x105,	CONV_CHARSET_VIETWAREX,	0},
	{_TEXT("Vietware F"),					1,		CONV_CHARSET_VIETWAREF,	0},
	{_TEXT("X UTF-8"),						0x101,	CONV_CHARSET_UNIUTF8,	UNICODE_UTF8_X},
	{_TEXT("Unicode C String"),				0x101,	CONV_CHARSET_UNI_CSTRING, UNICODE_CSTRING},
	{_TEXT("Unicode Muong"),				0x10A,	CONV_CHARSET_UNI_MUONG, UNICODE_UCS2}
};

////////////////////////////////////
// 1-byte charsets
////////////////////////////////////
unsigned char MapBD[][12][6] = {
//TCVN 3
    {{'¸','µ','¶','·','¹','a'},		// 0: a
    {'Ê','Ç','È','É','Ë','©'},		// 1: a^
    {'¾','»','¼','½','Æ','¨'},		// 2: a(
    {'Ð','Ì','Î','Ï','Ñ','e'},		// 3: e
    {'Õ','Ò','Ó','Ô','Ö','ª'},		// 4: e^
    {'Ý','×','Ø','Ü','Þ','i'},		// 5: i
    {'ã','ß','á','â','ä','o'},		// 6: o
    {'è','å','æ','ç','é','«'},		// 7: o^
    {'í','ê','ë','ì','î','¬'},		// 8: o+
    {'ó','ï','ñ','ò','ô','u'},		// 9: u
    {'ø','õ','ö','÷','ù','­'},		//10: u+ 
    {'ý','ú','û','ü','þ','y'}},		//11: y
//Vietware-F
    {{'À','ª','¶','º','Á','a'},
    {'Ê','Ç','È','É','Ë','¡'},
    {'Å','Â','Ã','Ä','Æ','Ÿ'},
    {'Ï','Ì','Í','Î','Ñ','e'},
    {'Õ','Ò','Ó','Ô','Ö','£'},
    {'Û','Ø','Ù','Ú','Ü','i'},
    {'â','ß','à','á','ã','o'},
    {'ç','ä','å','æ','è','¤'},
    {'ì','é','ê','ë','í','¥'},
    {'ò','î','ï','ñ','ó','u'},
    {'÷','ô','õ','ö','ø','§'},
    {'ü','ù','ú','û','ÿ','y'}}
};


unsigned char MapBK[][8] = {
	{'®', '§', 'ª', '£', '©', '¢', '«', '¤'},   //TCVN3
	{'¢', '˜', '£', '™', '¡', '—', '¤', 'š'}    //Vietware-F
};


unsigned char MapBW[][6] = {
	{'¨', '¡', '¬', '¥', '­', '¦'},   //TCVNI3
	{'Ÿ', '–', '¥', '›', '§', 'œ'}    //Vietware-F
};

unsigned char MapBT[][6] = {  // this tables must correspond to TelexShortcuts table
					{'¬', '­', '­', '¦', '¥', '¦'},	//TCVN3
					{'¥', '§', '§', 'œ', '¦', '§'}	//Vietware-F
};

unsigned char MapBTMuong[][6] = {  // this tables must correspond to TelexShortcutsForMuong table
	{ '¬', '­','¨', '¥', '¦', '¡' },	//TCVN3
	{ '¥', '§', '§', 'œ', '¦', '§' }	//Vietware-F
};
/*
unsigned char VnLayout[]={'ã', 'â', 'ê', 'ô', 'Ì', 'Ò', 'Þ', 'ì', 'ò', 'ð', 'þ', 'ý', 'õ',
                          'Ã', 'Â', 'Ê', 'Ô', 'Ð', 
                          

unsigned char USLayout[]={'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '=', '[', ']',
                          '!', '@', '#', '$', ')'
*/

//////////////////////////////////////////////////
// Unicode and 2-byte charsets
//////////////////////////////////////////////////

// Lower case
WORD UniVnL[][72] = {
//Unicode
	{0x00e1, 0x00e0, 0x1ea3, 0x00e3, 0x1ea1, 0x0061,
	 0x1ea5, 0x1ea7, 0x1ea9, 0x1eab, 0x1ead, 0x00e2,
	 0x1eaf, 0x1eb1, 0x1eb3, 0x1eb5, 0x1eb7, 0x0103,
	 0x00e9, 0x00e8, 0x1ebb, 0x1ebd, 0x1eb9, 0x0065,
	 0x1ebf, 0x1ec1, 0x1ec3, 0x1ec5, 0x1ec7, 0x00ea,
	 0x00ed, 0x00ec, 0x1ec9, 0x0129, 0x1ecb, 0x0069,
	 0x00f3, 0x00f2, 0x1ecf, 0x00f5, 0x1ecd, 0x006f,
	 0x1ed1, 0x1ed3, 0x1ed5, 0x1ed7, 0x1ed9, 0x00f4,
	 0x1edb, 0x1edd, 0x1edf, 0x1ee1, 0x1ee3, 0x01a1, 
	 0x00fa, 0x00f9, 0x1ee7, 0x0169, 0x1ee5, 0x0075,
	 0x1ee9, 0x1eeb, 0x1eed, 0x1eef, 0x1ef1, 0x01b0, 
	 0x00fd, 0x1ef3, 0x1ef7, 0x1ef9, 0x1ef5, 0x0079},
//VNI
	{0xf961, 0xf861, 0xfb61, 0xf561, 0xef61, 0x0061,
	 0xe161, 0xe061, 0xe561, 0xe361, 0xe461, 0xe261,
	 0xe961, 0xe861, 0xfa61, 0xfc61, 0xeb61, 0xea61,
	 0xf965, 0xf865, 0xfb65, 0xf565, 0xef65, 0x0065,
	 0xe165, 0xe065, 0xe565, 0xe365, 0xe465, 0xe265,
	 0x00ed, 0x00ec, 0x00e6, 0x00f3, 0x00f2, 0x0069,
	 0xf96f, 0xf86f, 0xfb6f, 0xf56f, 0xef6f, 0x006f,
	 0xe16f, 0xe06f, 0xe56f, 0xe36f, 0xe46f, 0xe26f,
	 0xf9f4, 0xf8f4, 0xfbf4, 0xf5f4, 0xeff4, 0x00f4,
	 0xf975, 0xf875, 0xfb75, 0xf575, 0xef75, 0x0075,
	 0xf9f6, 0xf8f6, 0xfbf6, 0xf5f6, 0xeff6, 0x00f6,
	 0xf979, 0xf879, 0xfb79, 0xf579, 0x00ee, 0x0079},
//BKHCM2
	{0xe161, 0xe261, 0xe361, 0xe461, 0xe561, 0x0061,
	 0xebea, 0xecea, 0xEDEA, 0xEEEA, 0xE5EA, 0x00EA,
	 0xE6F9, 0xE7F9, 0xE8F9, 0xE9F9, 0xE5F9, 0x00F9,
	 0xE165, 0xE265, 0xE365, 0xE465, 0xE565, 0x0065,
	 0xEBEF, 0xECEF, 0xEDEF, 0xEEEF, 0xE5EF, 0x00EF,
	 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x0069,
	 0xE16F, 0xE26F, 0xE36F, 0xE46F, 0xE56F, 0x006F,
	 0xEBF6, 0xECF6, 0xEDF6, 0xEEF6, 0xE5F6, 0x00F6,
	 0xE1FA, 0xE2FA, 0xE3FA, 0xE4FA, 0xE5FA, 0x00FA,
	 0xE175, 0xE275, 0xE375, 0xE475, 0xE575, 0x0075,
	 0xE1FB, 0xE2FB, 0xE3FB, 0xE4FB, 0xE5FB, 0x00FB,
	 0xE179, 0xE279, 0xE379, 0xE479, 0xE579, 0x0079},
//VPS
	{0x00E1, 0x00E0, 0x00E4, 0x00E3, 0x00E5, 0x0061,
	 0x00C3, 0x00C0, 0x00C4, 0x00C5, 0x00C6, 0x00E2,
	 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00E6,
	 0x00E9, 0x00E8, 0x00C8, 0x00EB, 0x00CB, 0x0065,
	 0x0089, 0x008A, 0x008B, 0x00CD, 0x008C, 0x00EA,
	 0x00ED, 0x00EC, 0x00CC, 0x00EF, 0x00CE, 0x0069,
	 0x00F3, 0x00F2, 0x00D5, 0x00F5, 0x0086, 0x006F,
	 0x00D3, 0x00D2, 0x00B0, 0x0087, 0x00B6, 0x00F4,
	 0x00A7, 0x00A9, 0x00AA, 0x00AB, 0x00AE, 0x00D6,
	 0x00FA, 0x00F9, 0x00FB, 0x00DB, 0x00F8, 0x0075,
	 0x00D9, 0x00D8, 0x00BA, 0x00BB, 0x00BF, 0x00DC,
	 0x009A, 0x00FF, 0x009B, 0x00CF, 0x009C, 0x0079},
//VIETWARE X
	{0xEF61, 0xEC61, 0xED61, 0xEE61, 0xFB61, 0x0061,
	 0xFAE1, 0xF6E1, 0xF8E1, 0xF9E1, 0xFBE1, 0x00E1,
	 0xF5E0, 0xF2E0, 0xF3E0, 0xF4E0, 0xFBE0, 0x00E0,
	 0xEF65, 0xEC65, 0xED65, 0xEE65, 0xFB65, 0x0065,
	 0xFAE3, 0xF6E3, 0xF8E3, 0xF9E3, 0xFBE3, 0x00E3,
	 0x00EA, 0x00E7, 0x00E8, 0x00E9, 0x00EB, 0x0069,
	 0xEF6F, 0xEC6F, 0xED6F, 0xEE6F, 0xFC6F, 0x006F,
	 0xFAE4, 0xF6E4, 0xF8E4, 0xF9E4, 0xFCE4, 0x00E4,
	 0xEFE5, 0xECE5, 0xEDE5, 0xEEE5, 0xFCE5, 0x00E5,
	 0xEF75, 0xEC75, 0xED75, 0xEE75, 0xFB75, 0x0075,
	 0xEFE6, 0xECE6, 0xEDE6, 0xEEE6, 0xFBE6, 0x00E6,
	 0xEF79, 0xEC79, 0xED79, 0xEE79, 0xF179, 0x0079},
//VISCII
	{0x00E1, 0x00E0, 0x00E4, 0x00E3, 0x00D5, 0x0061,
	 0x00A4, 0x00A5, 0x00A6, 0x00E7, 0x00A7, 0x00E2,
	 0x00A1, 0x00A2, 0x00C6, 0x00C7, 0x00A3, 0x00E5,
	 0x00E9, 0x00E8, 0x00EB, 0x00A8, 0x00A9, 0x0065,
	 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00EA,
	 0x00ED, 0x00EC, 0x00EF, 0x00EE, 0x00B8, 0x0069,
	 0x00F3, 0x00F2, 0x00F6, 0x00F5, 0x00F7, 0x006F,
	 0x00AF, 0x00B0, 0x00B1, 0x00B2, 0x00B5, 0x00F4,
	 0x00BE, 0x00B6, 0x00B7, 0x00DE, 0x00FE, 0x00BD,
	 0x00FA, 0x00F9, 0x00FC, 0x00FB, 0x00F8, 0x0075,
	 0x00D1, 0x00D7, 0x00D8, 0x00E6, 0x00F1, 0x00DF,
	 0x00FD, 0x00CF, 0x00D6, 0x00DB, 0x00DC, 0x0079},
//BKHCM1
	{0x00BE, 0x00BF, 0x00C0, 0x00C1, 0x00C2, 0x0061,
	 0x00DE, 0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x00DD,
	 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00D7,
	 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x0065,
	 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E3,
	 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x0069,
	 0x00CD, 0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x006F,
	 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00E9,
	 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00EF,
	 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0075,
	 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00F5,
	 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF, 0x0079},
//composed unicode
// composite bases that are greater than FF
// 0: A( 0x102
// 1: a( 0x103
// 2: O+ 0x1A0
// 3: o+ 0x1A1
// 4: U+ 0x1AF
// 5: u+ 0x1B0
// 6: DD 0x110
// 7: dd 0x111
//
// Use high byte as the index to the tone table below:
// 1: '
// 2: `
// 3: ?
// 4: ~
// 5: .
	{0x0161, 0x0261, 0x0361, 0x0461, 0x0561, 0x0061,  //a 0x0261
	 0x01e2, 0x02e2, 0x03e2, 0x04e2, 0x05e2, 0x00e2,  //a^
	 0x0101, 0x0201, 0x0301, 0x0401, 0x0501, 0x0001,  //a(
	 0x0165, 0x0265, 0x0365, 0x0465, 0x0565, 0x0065,  //e
	 0x01ea, 0x02ea, 0x03ea, 0x04ea, 0x05ea, 0x00ea,  //e^
	 0x0169, 0x0269, 0x0369, 0x0469, 0x0569, 0x0069,  //i
	 0x016f, 0x026f, 0x036f, 0x046f, 0x056f, 0x006f,  //o
	 0x01f4, 0x02f4, 0x03f4, 0x04f4, 0x05f4, 0x00f4,  //o^
	 0x0103, 0x0203, 0x0303, 0x0403, 0x0503, 0x0003,  //o+
	 0x0175, 0x0275, 0x0375, 0x0475, 0x0575, 0x0075,  //u
	 0x0105, 0x0205, 0x0305, 0x0405, 0x0505, 0x0005,  //u+
	 0x0179, 0x0279, 0x0379, 0x0479, 0x0579, 0x0079}, //y
// Windows CP 1258
	{0xec61, 0xcc61, 0xd261, 0xde61, 0xf261, 0x0061,  //a
	 0xece2, 0xcce2, 0xd2e2, 0xdee2, 0xf2e2, 0x00e2,  //a^
	 0xece3, 0xcce3, 0xd2e3, 0xdee3, 0xf2e3, 0x00e3,  //a(
	 0xec65, 0xcc65, 0xd265, 0xde65, 0xf265, 0x0065,  //e
	 0xecea, 0xccea, 0xd2ea, 0xdeea, 0xf2ea, 0x00ea,  //e^
	 0xec69, 0xcc69, 0xd269, 0xde69, 0xf269, 0x0069,  //i
	 0xec6f, 0xcc6f, 0xd26f, 0xde6f, 0xf26f, 0x006f,  //o
	 0xecf4, 0xccf4, 0xd2f4, 0xdef4, 0xf2f4, 0x00f4,  //o^
	 0xecf5, 0xccf5, 0xd2f5, 0xdef5, 0xf2f5, 0x00f5,  //o+
	 0xec75, 0xcc75, 0xd275, 0xde75, 0xf275, 0x0075,  //u
	 0xecfd, 0xccfd, 0xd2fd, 0xdefd, 0xf2fd, 0x00fd,  //u+
	 0xec79, 0xcc79, 0xd279, 0xde79, 0xf279, 0x0079}, //y
// Muong Uni
	 {0x0161, 0x0261, 0x0361, 0x0461, 0x0561, 0x0061,  //a 0x0261
	 0x01e2, 0x02e2, 0x03e2, 0x04e2, 0x05e2, 0x00e2,  //a^
	 0x0101, 0x0201, 0x0301, 0x0401, 0x0501, 0x0001,  //a(
	 0x0165, 0x0265, 0x0365, 0x0465, 0x0565, 0x0065,  //e
	 0x01ea, 0x02ea, 0x03ea, 0x04ea, 0x05ea, 0x00ea,  //e^
	 0x0169, 0x0269, 0x0369, 0x0469, 0x0569, 0x0069,  //i
	 0x016f, 0x026f, 0x036f, 0x046f, 0x056f, 0x006f,  //o
	 0x01f4, 0x02f4, 0x03f4, 0x04f4, 0x05f4, 0x00f4,  //o^
	 0x0103, 0x0203, 0x0303, 0x0403, 0x0503, 0x0003,  //o+
	 0x0175, 0x0275, 0x0375, 0x0475, 0x0575, 0x0075,  //u
	 0x0105, 0x0205, 0x0305, 0x0405, 0x0505, 0x0005,  //u+
	 0x0179, 0x0279, 0x0379, 0x0479, 0x0579, 0x0079 }
};


// Uppercase
WORD UniVnH[][72] = {
// Unicode
	{0x00c1, 0x00c0, 0x1ea2, 0x00c3, 0x1ea0, 0x0041,
	 0x1ea4, 0x1ea6, 0x1ea8, 0x1eaa, 0x1eac, 0x00c2,
	 0x1eae, 0x1eb0, 0x1eb2, 0x1eb4, 0x1eb6, 0x0102,
	 0x00c9, 0x00c8, 0x1eba, 0x1ebc, 0x1eb8, 0x0045,
	 0x1ebe, 0x1ec0, 0x1ec2, 0x1ec4, 0x1ec6, 0x00ca,
	 0x00cd, 0x00cc, 0x1ec8, 0x0128, 0x1eca, 0x0049,
	 0x00d3, 0x00d2, 0x1ece, 0x00d5, 0x1ecc, 0x004f,
	 0x1ed0, 0x1ed2, 0x1ed4, 0x1ed6, 0x1ed8, 0x00d4,
	 0x1eda, 0x1edc, 0x1ede, 0x1ee0, 0x1ee2, 0x01a0,
	 0x00da, 0x00d9, 0x1ee6, 0x0168, 0x1ee4, 0x0055,
	 0x1ee8, 0x1eea, 0x1eec, 0x1eee, 0x1ef0, 0x01af,
	 0x00dd, 0x1ef2, 0x1ef6, 0x1ef8, 0x1ef4, 0x0059},
// VNI
	{0xd941, 0xd841, 0xdb41, 0xd541, 0xcf41, 0x0041,
	 0xc141, 0xc041, 0xc541, 0xc341, 0xc441, 0xc241,
	 0xc941, 0xc841, 0xda41, 0xdc41, 0xcb41, 0xca41,
	 0xd945, 0xd845, 0xdb45, 0xd545, 0xcf45, 0x0045,
	 0xc145, 0xc045, 0xc545, 0xc345, 0xc445, 0xc245,
	 0x00cd, 0x00cc, 0x00c6, 0x00d3, 0x00d2, 0x0049,
	 0xd94f, 0xd84f, 0xdb4f, 0xd54f, 0xcf4f, 0x004f,
	 0xc14f, 0xc04f, 0xc54f, 0xc34f, 0xc44f, 0xc24f,
	 0xd9d4, 0xd8d4, 0xdbd4, 0xd5d4, 0xcfd4, 0x00d4,
	 0xd955, 0xd855, 0xdb55, 0xd555, 0xcf55, 0x0055,
	 0xd9d6, 0xd8d6, 0xdbd6, 0xd5d6, 0xcfd6, 0x00d6,
	 0xd959, 0xd859, 0xdb59, 0xd559, 0x00ce, 0x0059},
//BKHCM2
	{0xC141, 0xC241, 0xC341, 0xC441, 0xC541, 0x0041,
	 0xCBCA, 0xCCCA, 0xCDCA, 0xCECA, 0xC5CA, 0x00CA,
	 0xC6D9, 0xC7D9, 0xC8D9, 0xC9D9, 0xC5D9, 0x00D9,
	 0xC145, 0xC245, 0xC345, 0xC445, 0xC545, 0x0045,
	 0xCBCF, 0xCCCF, 0xCDCF, 0xCECF, 0xE5CF, 0x00CF,
	 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x0049,
	 0xC14F, 0xC24F, 0xC34F, 0xC44F, 0xC54F, 0x004F,
	 0xCBD6, 0xCCD6, 0xCDD6, 0xCED6, 0xC5D6, 0x00D6,
	 0xC1DA, 0xC2DA, 0xC3DA, 0xC4DA, 0xC5DA, 0x00DA,
	 0xC155, 0xC255, 0xC355, 0xC455, 0xC555, 0x0055,
	 0xC1DB, 0xC2DB, 0xC3DB, 0xC4DB, 0xC5DB, 0x00DB,
	 0xC159, 0xC259, 0xC359, 0xC459, 0xC559, 0x0059},
//VPS
	{0x00C1, 0x0080, 0x0081, 0x0082, 0x2E41, 0x0041,
	 0x0083, 0x0084, 0x0085, 0x7EC2, 0x2EC2, 0x00C2,
	 0x008D, 0x008E, 0x008F, 0x00F0, 0x2E88, 0x0088,
	 0x00C9, 0x00D7, 0x00DE, 0x00FE, 0x2E45, 0x0045,
	 0x0090, 0x0093, 0x0094, 0x0095, 0x2ECA, 0x00CA,
	 0x00B4, 0x00B5, 0x00B7, 0x00B8, 0x2E49, 0x0049,
	 0x00B9, 0x00BC, 0x00BD, 0x00BE, 0x2E4F, 0x004F,
	 0x0096, 0x0097, 0x0098, 0x0099, 0x2ED4, 0x00D4,
	 0x009D, 0x009E, 0x009F, 0x00A6, 0x2EF7, 0x00F7,
	 0x00DA, 0x00A8, 0x00D1, 0x00AC, 0x2E55, 0x0055,
	 0x00AD, 0x00AF, 0x00B1, 0x7ED0, 0x2ED0, 0x00D0,
	 0x00DD, 0x00B2, 0x00FD, 0x00B3, 0x2E59, 0x0059},
//VIETWARE X
	{0xCF41, 0xCC41, 0xCD41, 0xCE41, 0xDB41, 0x0041,
	 0xDAC1, 0xD6C1, 0xD8C1, 0xD9C1, 0xDBC1, 0x00C1,
	 0xD5C0, 0xD2C0, 0xD3C0, 0xD4C0, 0xDBC0, 0x00C0,
	 0xCF45, 0xCC45, 0xCD45, 0xCE45, 0xDB45, 0x0045,
	 0xDAC3, 0xD6C3, 0xD8C3, 0xD9C3, 0xDBC3, 0x00C3,
	 0x00CA, 0x00C7, 0x00C8, 0x00C9, 0x00CB, 0x0049,
	 0xCF4F, 0xCC4F, 0xCD4F, 0xCE4F, 0xDC4F, 0x004F,
	 0xDAC4, 0xD6C4, 0xD8C4, 0xD9C4, 0xDCC4, 0x00C4,
	 0xCFC5, 0xCCC5, 0xCDC5, 0xCEC5, 0xDCC5, 0x00C5,
	 0xCF55, 0xCC55, 0xCD55, 0xCE55, 0xDB55, 0x0055,
	 0xCFC6, 0xCCC6, 0xCDC6, 0xCEC6, 0xDBC6, 0x00C6,
	 0xCF59, 0xCC59, 0xCD59, 0xCE59, 0xD159, 0x0059},
//VISCII
	{0x00C1, 0x00C0, 0x00C4, 0x00C3, 0x0080, 0x0041,
	 0x0084, 0x0085, 0x0086, 0x00E7, 0x0087, 0x00C2,
	 0x0081, 0x0082, 0x00C6, 0x00C7, 0x0083, 0x00C5,
	 0x00C9, 0x00C8, 0x00CB, 0x0088, 0x0089, 0x0045,
	 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x00CA,
	 0x00CD, 0x00CC, 0x009B, 0x00CE, 0x0098, 0x0049,
	 0x00D3, 0x00D2, 0x0099, 0x00F5, 0x009A, 0x004F,
	 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x00D4,
	 0x0095, 0x0096, 0x0097, 0x00B3, 0x0094, 0x00B4,
	 0x00DA, 0x00D9, 0x009C, 0x009D, 0x009E, 0x0055,
	 0x00BA, 0x00BB, 0x00BC, 0x00FF, 0x00B9, 0x00BF,
	 0x00DD, 0x009F, 0x00D6, 0x00DB, 0x00DC, 0x0059},
//BKHCM1
	{0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0041,
	 0x007E, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x009F,
	 0x009A, 0x009B, 0x009C, 0x009D, 0x0098, 0x0099,
	 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x0045,
	 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00A5,
	 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x0049,
	 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x004F,
	 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00AB,
	 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B1,
	 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0055,
	 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00B7,
	 0x007B, 0x005E, 0x0060, 0x007C, 0x008E, 0x0059},
//unicode compound
//unicode compound
// composite bases that are greater than FF
// 0: A( 0x102
// 1: a( 0x103
// 2: O+ 0x1A0
// 3: o+ 0x1A1
// 4: U+ 0x1AF
// 5: u+ 0x1B0
// 6: DD 0x110
// 7: dd 0x111
//
// Use high byte is the index to the tone table below:
// 1: '
// 2: `
// 3: ?
// 4: ~
// 5: .
	{0x0141, 0x0241, 0x0341, 0x0441, 0x0541, 0x0041,  //a
	 0x01c2, 0x02c2, 0x03c2, 0x04c2, 0x05c2, 0x00c2,  //a^
	 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0000,  //a(
	 0x0145, 0x0245, 0x0345, 0x0445, 0x0545, 0x0045,  //e
	 0x01ca, 0x02ca, 0x03ca, 0x04ca, 0x05ca, 0x00ca,  //e^
	 0x0149, 0x0249, 0x0349, 0x0449, 0x0549, 0x0049,  //i
	 0x014f, 0x024f, 0x034f, 0x044f, 0x054f, 0x004f,  //o
	 0x01d4, 0x02d4, 0x03d4, 0x04d4, 0x05d4, 0x00d4,  //o^
	 0x0102, 0x0202, 0x0302, 0x0402, 0x0502, 0x0002,  //o+
	 0x0155, 0x0255, 0x0355, 0x0455, 0x0555, 0x0055,  //u
	 0x0104, 0x0204, 0x0304, 0x0404, 0x0504, 0x0004,  //u+
	 0x0159, 0x0259, 0x0359, 0x0459, 0x0559, 0x0059}, //y
// Windows CP 1258
	{0xec41, 0xcc41, 0xd241, 0xde41, 0xf241, 0x0041, //a
	 0xecc2, 0xccc2, 0xd2c2, 0xdec2, 0xf2c2, 0x00c2, //a^
	 0xecc3, 0xccc3, 0xd2c3, 0xdec3, 0xf2c3, 0x00c3, //a(
	 0xec45, 0xcc45, 0xd245, 0xde45, 0xf245, 0x0045, //e
	 0xecca, 0xccca, 0xd2ca, 0xdeca, 0xf2ca, 0x00ca, //e^
	 0xec49, 0xcc49, 0xd249, 0xde49, 0xf249, 0x0049, //i
	 0xec4f, 0xcc4f, 0xd24f, 0xde4f, 0xf24f, 0x004f, //o
	 0xecd4, 0xccd4, 0xd2d4, 0xded4, 0xf2d4, 0x00d4, //o^
	 0xecd5, 0xccd5, 0xd2d5, 0xded5, 0xf2d5, 0x00d5, //o+
	 0xec55, 0xcc55, 0xd255, 0xde55, 0xf255, 0x0055, //u
	 0xecdd, 0xccdd, 0xd2dd, 0xdedd, 0xf2dd, 0x00dd, //u+
	 0xec59, 0xcc59, 0xd259, 0xde59, 0xf259, 0x0059}, //y
// Muong Uni
	 {0x0141, 0x0241, 0x0341, 0x0441, 0x0541, 0x0041,  //a
	 0x01c2, 0x02c2, 0x03c2, 0x04c2, 0x05c2, 0x00c2,  //a^
	 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0000,  //a(
	 0x0145, 0x0245, 0x0345, 0x0445, 0x0545, 0x0045,  //e
	 0x01ca, 0x02ca, 0x03ca, 0x04ca, 0x05ca, 0x00ca,  //e^
	 0x0149, 0x0249, 0x0349, 0x0449, 0x0549, 0x0049,  //i
	 0x014f, 0x024f, 0x034f, 0x044f, 0x054f, 0x004f,  //o
	 0x01d4, 0x02d4, 0x03d4, 0x04d4, 0x05d4, 0x00d4,  //o^
	 0x0102, 0x0202, 0x0302, 0x0402, 0x0502, 0x0002,  //o+
	 0x0155, 0x0255, 0x0355, 0x0455, 0x0555, 0x0055,  //u
	 0x0104, 0x0204, 0x0304, 0x0404, 0x0504, 0x0004,  //u+
	 0x0159, 0x0259, 0x0359, 0x0459, 0x0559, 0x0059} //y
};


// Character d-
WORD UniVndd[] = {
					0x0111,   // Unicode
					0x00f1,   // VNI
					0x00E0,   // BKHCM2
					0x00C7,   // VPS
					0x00E2,   // VIETWARE X
					0x00F0,   // VISCII
					0x00BD,   // BKHCM1
					0x0007,   // unicode compound 
					0x00f0,    // Windows CP 1258
					0x0007 // Uni Muong
};



// Character -D
WORD UniVnDD[] = {
					0x0110,  // Unicode
					0x00d1,  // VNI
					0x00C0,  // BKHCM2
					0x00F1,  // VPS
					0x00C2,  // VIETWARE X
					0x00D0,  // VISCII
					0x007D,  // BKHCM1
					0x0006,  // unicode compound
					0x00d0,  // Windows CP 1258
					0x0006 //Uni Muong
};


///// End of charset specification ///////////////////
//////////////////////////////////////////////////////

// Base characters of "doule" characters (like a^, a(, u+ )
// the order here must not be changed
unsigned char DoubleChars[] = {'d', 'D', 'e', 'E', 'a', 'A', 'o', 'O', 'u', 'U'};

unsigned char TelexTones[6] = {'s', 'f', 'r', 'x', 'j', 'z'};
unsigned char TelexBreves[] = {'w', 'W'};					// Da^'u tra(ng (breve mark)
unsigned char TelexDuplicate[] = { 'q', 'Q' };				// Nhan doi nguyen am (duplicate_mark)
unsigned char TelexShortcuts[] = {'[', ']', 'w', 'W', '{', '}'};

unsigned char TelexShortcutsForMuong[] = { '[', ']','\\', '{', '}','|'};


unsigned char ToneLimits[] = {
	'b', 'd', 'f', 'j', 'k', 'l', 'q', 'r', 's', 'v', 'w', 'x', 'z',
	'B', 'D', 'F', 'J', 'K', 'L', 'Q', 'R', 'S', 'V', 'W', 'X', 'Z', 
	',', ';', ':', '.', '\"', '\'', '!', '?', 
	' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'<', '>', '=', '+', '-', '*', '/', '\\',
	'_', '~', '`', '@', '#', '$', '%', '^', '&', '(', ')', '{', '}', '[', ']'};

unsigned char ToneLimitsForMuong[] = {
	'b', 'd', 'f', 'j', 'k', 'q', 'r', 's', 'v', 'x', 'z',
	'B', 'D', 'F', 'J', 'K', 'Q', 'R', 'S', 'V', 'X', 'Z',
	',', ';', ':', '.', '\"', '\'', '!', '?',
	' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'<', '>', '=', '+', '-', '*', '/', '\\',
	'_', '~', '`', '@', '#', '$', '%', '^', '&', '(', ')', '{', '}', '[', ']' };

unsigned char WordStops[] = {
	',', ';', ':', '.', '\"', '\'', '!', '?', ' ',
	'<', '>', '=', '+', '-', '*', '/', '\\',
	'_', '~', '`', '@', '#', '$', '%', '^', '&', '(', ')', '{', '}', '[', ']'};

// VIQR charset
unsigned char VIQRtones[] = {'\'','`','?','~','.', 0};
unsigned char VIQRcircumflex = '^';
unsigned char VIQRbreve = '(';
unsigned char VIQRhorn = '+';

// VNI-like (VNI, VIQR, VIQR*) input keys
unsigned char VniToneKeys[] = {'1', '2', '3', '4', '5', '0'};
unsigned char VniDoubleKeys[] = {'6', '7', '8', '9'};

unsigned char VIQRToneKeys[] = {'\'', '`', '?', '~', '.', '0'};
unsigned char VIQRDoubleKeys[] = {'^', '+', '(', 'd'};

unsigned char VIQRStarToneKeys[] = {'\'', '`', '?', '~', '.', '0'};
unsigned char VIQRStarDoubleKeys[] = {'^', '*', '(', 'd'};



void BuildTelexMethod(CodeInfo *pTab);
void BuildVniLikeMethod(CodeInfo *pTab, int method); //vniLike = 1 for VIQR mode
void BuildTelexWithMuongMethod(CodeInfo *pTab);
void BuildInputMethod(int method, CodeInfo *pTab)
{
	switch (method)
	{
	case TELEX_INPUT:
		BuildTelexMethod(pTab);
		break;
	case TELEX_MUONG:
		BuildTelexWithMuongMethod(pTab);
		break;
	case VNI_INPUT:
		BuildVniLikeMethod(pTab, method);
		break;
	default:
		BuildTelexMethod(pTab);
		break;
	}
}

void BuildTelexMethod(CodeInfo *pTab)
{
	long i, j, offset;
	unsigned char ch;

	for (i=0; i<256; i++)
		pTab->DT[i] = 0x2000000; // bit 25: hard separator

	for (ch = 'a'; ch <= 'z'; ch++)
		pTab->DT[ch] = 0;

	for (ch = 'A'; ch <= 'Z'; ch++)
		pTab->DT[ch] = 0;

	for (i = 0; i < sizeof(ToneLimits); i++)
		pTab->DT[ToneLimits[i]] = 0;

	for (i = 0; i < sizeof(pTab->BK); i++)
		pTab->DT[pTab->BK[i]] = 0;

	for (i = 0; i < sizeof(pTab->BW); i++)
		pTab->DT[pTab->BW[i]] = 0;

	for (i=0; i<12; i++) { // we have 12 vowel sets
		for (j=0; j<6; j++) {// each vowel set has 6
			pTab->DT[pTab->BD[i][j]] = i+1;
			pTab->DT[pTab->BD[i][j]] |= (j+1) << 18; // tone index (from bit 18)
		}
	}

	// Set index for for characters like: dd, or a(, 
	offset = sizeof(DoubleChars) - sizeof(pTab->BW);
	for (i=0; i < sizeof(DoubleChars); i++) {
		pTab->DT[DoubleChars[i]] |= ((i+1) << 9);
		if (i < sizeof(pTab->BK))
			pTab->DT[pTab->BK[i]] |= ((i+1) << 9);
		if (i+1 > offset)
			pTab->DT[pTab->BW[i-offset]] |= ((i+1) << 9);
	}

	// Set tone limits (soft separator)
	for (i = 0; i < sizeof(ToneLimits); i++)
		pTab->DT[ToneLimits[i]] |= 0x1000000;  // Set bit 24 for soft separators

	// Set word stops
	for (i = 0; i < sizeof(WordStops); i++)
		pTab->DT[WordStops[i]] |= 0x20000000;  // Set bit 29 for word stops

	// For tone makrs
	for (i=0; i < sizeof(TelexTones); i++) {
		pTab->DT[TelexTones[i]] |= (i+1) << 14; // tone mark from bit 14 to 17
		pTab->DT[toupper(TelexTones[i])] |= pTab->DT[TelexTones[i]];
	}

	// For the mark ( or +
	for (i=0; i < sizeof(TelexBreves); i++)
		pTab->DT[TelexBreves[i]] |= 0x400000; // set bit 22 for the breve mark

	// For macro keys
	for (i=0; i < sizeof(TelexShortcuts); i++)
		pTab->DT[TelexShortcuts[i]] |= (i+1) << 5; // macro keys from bit 5
}
//-------------------------------------------
void BuildTelexWithMuongMethod(CodeInfo *pTab)
{
	long i, j, offset;
	unsigned char ch;

	for (i = 0; i<256; i++)
		pTab->DT[i] = 0x2000000; // bit 25: hard separator

	for (ch = 'a'; ch <= 'z'; ch++)
		pTab->DT[ch] = 0;

	for (ch = 'A'; ch <= 'Z'; ch++)
		pTab->DT[ch] = 0;

	for (i = 0; i < sizeof(ToneLimits); i++)
		pTab->DT[ToneLimits[i]] = 0;

	for (i = 0; i < sizeof(pTab->BK); i++)
		pTab->DT[pTab->BK[i]] = 0;

	for (i = 0; i < sizeof(pTab->BW); i++)
		pTab->DT[pTab->BW[i]] = 0;

	for (i = 0; i<12; i++) { // we have 12 vowel sets
		for (j = 0; j<6; j++) {// each vowel set has 6
			pTab->DT[pTab->BD[i][j]] = i + 1;
			pTab->DT[pTab->BD[i][j]] |= (j + 1) << 18; // tone index (from bit 18)
		}
	}

	// Set index for for characters like: dd, or a(, 
	offset = sizeof(DoubleChars) - sizeof(pTab->BW);
	for (i = 0; i < sizeof(DoubleChars); i++) {
		pTab->DT[DoubleChars[i]] |= ((i + 1) << 9);
		if (i < sizeof(pTab->BK))
			pTab->DT[pTab->BK[i]] |= ((i + 1) << 9);
		if (i + 1 > offset)
			pTab->DT[pTab->BW[i - offset]] |= ((i + 1) << 9);
	}

	// Set tone limits (soft separator)
	for (i = 0; i < sizeof(ToneLimitsForMuong); i++)
		pTab->DT[ToneLimitsForMuong[i]] |= 0x1000000;  // Set bit 24 for soft separators

											   // Set word stops
	for (i = 0; i < sizeof(WordStops); i++)
		pTab->DT[WordStops[i]] |= 0x20000000;  // Set bit 29 for word stops

											   // For tone makrs
	for (i = 0; i < sizeof(TelexTones); i++) {
		pTab->DT[TelexTones[i]] |= (i + 1) << 14; // tone mark from bit 14 to 17
		pTab->DT[toupper(TelexTones[i])] |= pTab->DT[TelexTones[i]];
	}

	// For the mark ( or +
	for (i = 0; i < sizeof(TelexBreves); i++)
		pTab->DT[TelexBreves[i]] |= 0x400000; // set bit 22 for the breve mark
											  // For the mark ( or +

	for (i = 0; i < sizeof(TelexDuplicate); i++)
		pTab->DT[TelexDuplicate[i]] |= 0x800000; // set bit 23 for the duplicate mark

											  // For macro keys
	//for (i = 0; i < sizeof(TelexShortcuts); i++)
	//	pTab->DT[TelexShortcuts[i]] |= (i + 1) << 5; // macro keys from bit 5

	for (i = 0; i < sizeof(TelexShortcutsForMuong); i++)
		pTab->DT[TelexShortcutsForMuong[i]] |= (i + 1) << 5; // macro keys from bit 5
}
//-------------------------------------------
void BuildVniLikeMethod(CodeInfo *pTab, int method)
{
	int i, j;
	unsigned char ch;
	int offset;

	for (i=0; i<256; i++)
		pTab->DT[i] = 0x2000000; // bit 25: hard separator

	for (ch = 'a'; ch <= 'z'; ch++)
		pTab->DT[ch] = 0;

	for (ch = 'A'; ch <= 'Z'; ch++)
		pTab->DT[ch] = 0;

	for (i = 0; i < sizeof(ToneLimits); i++)
		pTab->DT[ToneLimits[i]] = 0;

	for (i = 0; i < sizeof(pTab->BK); i++)
		pTab->DT[pTab->BK[i]] = 0;

	for (i = 0; i < sizeof(pTab->BW); i++)
		pTab->DT[pTab->BW[i]] = 0;

	for (i=0; i<12; i++) { // we have 12 vowel sets
		for (j=0; j<6; j++) {
			// each vowel set has 6
			pTab->DT[pTab->BD[i][j]] = i+1;
			pTab->DT[pTab->BD[i][j]] |= (j+1) << 18; // tone index (from bit 18)
		}
	}

	// For characters like: dd, or a(, 
	offset = sizeof(DoubleChars) - sizeof(pTab->BW);
	for (i=0; i < sizeof(DoubleChars); i++) {
		pTab->DT[DoubleChars[i]] |= ((i+1) << 9);
		if (i < sizeof(pTab->BK))
			pTab->DT[pTab->BK[i]] |= ((i+1) << 9);
		if (i+1 > offset)
			pTab->DT[pTab->BW[i-offset]] |= ((i+1) << 9);
	}

	// Set tone limits
	for (i = 0; i < sizeof(ToneLimits); i++)
		pTab->DT[ToneLimits[i]] |= 0x1000000;  // Set bit 24 for soft separator

	// Set word stops
	for (i = 0; i < sizeof(WordStops); i++)
		pTab->DT[WordStops[i]] |= 0x20000000;  // Set bit 29 for word stops

	unsigned char *doubleKeys;
	unsigned char *toneKeys;
	if (method == VNI_INPUT) {
		doubleKeys = VniDoubleKeys;
		toneKeys = VniToneKeys;
	} else if (method == VIQR_INPUT) {
		doubleKeys = VIQRDoubleKeys;
		toneKeys = VIQRToneKeys;
	} else { //VIQR*
		doubleKeys = VIQRStarDoubleKeys;
		toneKeys = VIQRStarToneKeys;
	}

	for (j=0; j < sizeof(VniDoubleKeys); j++)
		pTab->DT[doubleKeys[j]] |= (j + 1) << 26; // VNI-like double marks: from bit 26 - 28
	for (i=0; i < sizeof(VniToneKeys); i++)
		pTab->DT[toneKeys[i]] |= (i+1) << 14;  // VNI-like tone marks from bit 14
}


//---------------------------------------------
void BuildCodeTable(WORD charSet, int inputMethod, CodeInfo *pTab)
{
	int index;
	WORD i, j;
	unsigned char ch, up;

	index = charSet;
	if (charSet >= 0x100) {
		index = TCVN3_CHARSET;
		pTab->multiBytes = 1;
	}
	else pTab->multiBytes = 0;
	//switch (charSet)
	//{
	//default:
	//	break;
	//}
	//CONV_CHARSET_TCVN3
	memcpy(pTab->BD, MapBD[index], sizeof(pTab->BD));
	memcpy(pTab->BK, MapBK[index], sizeof(pTab->BK));
	memcpy(pTab->BT, MapBT[index], sizeof(pTab->BT));
	memcpy(pTab->BW, MapBW[index], sizeof(pTab->BW));
	memcpy(pTab->BM, MapBTMuong[index], sizeof(pTab->BM));

	BuildInputMethod(inputMethod, pTab);
	//------- Build conversion table from TCVN3 to 2-bytes charsets -------
	if (charSet == VIQR_CHARSET) 
	{
		for (i = 0; i < 256; i++) {
			pTab->ToUniL[i] = i;
			pTab->ToUniH[i] = i;
		}
		for (ch= 'a', up = 'A'; ch <= 'z'; ch++, up++)
			pTab->ToUniH[ch] = up;
		for (i=0; i<12; i++) { // we have 12 vowel sets
			for (j=0; j<6; j++) {
				pTab->ToUniL[pTab->BD[i][j]] = MAKEWORD(pTab->BD[i][5], VIQRtones[j]);
				pTab->ToUniH[pTab->BD[i][j]] = MAKEWORD(toupper(pTab->BD[i][5]), VIQRtones[j]);
			}
		}

		pTab->ToUniL[pTab->BK[0]] = MAKEWORD('d', 'd');
		pTab->ToUniH[pTab->BK[0]] = MAKEWORD('D', 'D');
		for (i = 1; i < sizeof(pTab->BK); i++) {
			pTab->ToUniL[pTab->BK[i]] = MAKEWORD(DoubleChars[i], VIQRcircumflex);
			pTab->ToUniH[pTab->BK[i]] = MAKEWORD(toupper(DoubleChars[i]), VIQRcircumflex);
		}

		int offset = sizeof(DoubleChars) - sizeof(pTab->BW);
		for (i = 0; i < sizeof(pTab->BW); i++) {
			pTab->ToUniL[pTab->BW[i]] = MAKEWORD(DoubleChars[i+offset], (i < 2)? VIQRbreve : VIQRhorn);
			pTab->ToUniH[pTab->BW[i]] = MAKEWORD(toupper(DoubleChars[i+offset]), (i < 2)? VIQRbreve : VIQRhorn);
		}
	}
	else if (pTab->multiBytes) 
	{
		index = charSet - 0x100 - 1; // the first index is assigned to VIQR
		for (i=0; i<256; i++) {
			pTab->ToUniH[i] = i;
			pTab->ToUniL[i] = i;
		}

		for (ch= 'a', up = 'A'; ch <= 'z'; ch++, up++)
			pTab->ToUniH[ch] = up;

		unsigned char *p = (unsigned char *)pTab->BD;
		for (i=0; i<72; i++) {
			pTab->ToUniH[*p] = UniVnH[index][i];
			pTab->ToUniL[*p] = UniVnL[index][i];
			p++;
		}
		pTab->ToUniL[pTab->BK[0]] = UniVndd[index];
		pTab->ToUniH[pTab->BK[0]] = UniVnDD[index];

		pTab->ToUniL[pTab->BT[4]] = UniVnL[index][8*6+5]; // upper O+ (= BD[8][5])
		pTab->ToUniH[pTab->BT[4]] = UniVnH[index][8*6+5];

		pTab->ToUniL[pTab->BT[5]] = UniVnL[index][10*6+5]; // upper U+ (= BD[10][5])
		pTab->ToUniH[pTab->BT[5]] = UniVnH[index][10*6+5];

		pTab->ToUniL[pTab->BM[3]] = UniVnL[index][8 * 6 + 5]; // upper O+ (= BD[8][5])
		pTab->ToUniH[pTab->BM[3]] = UniVnH[index][8 * 6 + 5];

		pTab->ToUniL[pTab->BM[4]] = UniVnL[index][10 * 6 + 5]; // upper U+ (= BD[10][5])
		pTab->ToUniH[pTab->BM[4]] = UniVnH[index][10 * 6 + 5];

		pTab->ToUniL[pTab->BM[5]] = UniVnL[index][2 * 6 + 5]; // upper A( (= BD[2][5])
		pTab->ToUniH[pTab->BM[5]] = UniVnH[index][2 * 6 + 5];
	}
}
