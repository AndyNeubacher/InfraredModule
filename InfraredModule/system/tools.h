/*
 * tools.h
 *
 * Created: 04.11.2012 18:10:15
 *  Author: neubi
 */


#ifndef TOOLS_H_
#define TOOLS_H_


#define BSWAP_16(x)     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))



BYTE Ascii2Nibble(BYTE chr);

BYTE Nibble2Ascii(BYTE n);

void Ascii2Hex (BYTE *ascii_string, BYTE* target, BYTE len);

BYTE* Word2AsciiHex(WORD wrd, BYTE* buf);

BYTE* Byte2AsciiHex(BYTE bte, BYTE* buf);




#endif
