/*
 * tools.c
 *
 * Created: 04.11.2012 18:09:57
 *  Author: neubi
 */


#include "main.h"
#include "tools.h"



/**
*  converts one ASCII-char to one HEX-nibble
*
* @param  ASCII character
* @return value of ASCII character in HEX
*/
BYTE Ascii2Nibble(BYTE chr)
{
    if((chr >= '0') && (chr <= '9'))
        return (chr - '0');

    if((chr >= 'A') && (chr <= 'F'))
        return (chr-'A'+10);

    return 0xFF;      // invalid
}


/**
*  converts one nibble to one ASCII-Hex-Char
*
* @param  nibble (4bits)
* @return ASCII-character
*/
BYTE Nibble2Ascii(BYTE n)
{
    n &= 0x0F;              // mask out upper 4 bits

    if(n<10)
        return(n+'0');        // return digits
    else
        return((n-10)+'A');   // return characters
}


/**
*  converts an ASCII string to HEX values
*
* @param  ptr to ASCII string
* @param  ptr where result should be stored
* @param  nr of bytes to convert
* @return none
*/
void Ascii2Hex (BYTE *ascii_string, BYTE* target, BYTE len)
{
    while(len--)
    {
        *target    = (Ascii2Nibble(*ascii_string++) << 4);
        *target++ |= (Ascii2Nibble(*ascii_string++));
    }
}


/**
*  converts one WORD-value to 4BYTE-Hex-Text
*
*  @param  WORD-value
*  @param  pointer where Ascii-chars should be stored
*  @return pointer where Ascii-chars begin
*/
BYTE* Word2AsciiHex(WORD wrd, BYTE* buf)
{
    BYTE* ret = buf;

    *buf++ = Nibble2Ascii((BYTE)(wrd >> 12));
    *buf++ = Nibble2Ascii((BYTE)(wrd >> 8));
    *buf++ = Nibble2Ascii((BYTE)(wrd >> 4));
    *buf++ = Nibble2Ascii((BYTE)(wrd));
    *buf   = 0x00;

    return ret;
}


/**
*  converts one BYTE-value to 2BYTE-Hex-Text
*
*  @param  BYTE-value
*  @param  pointer where Ascii-chars should be stored
*  @return pointer where Ascii-chars begin
*/
BYTE* Byte2AsciiHex(BYTE bte, BYTE* buf)
{
    BYTE* ret = buf;

    *buf++ = Nibble2Ascii((BYTE)(bte >> 4));
    *buf++ = Nibble2Ascii((BYTE)(bte));
    *buf   = 0x00;

    return ret;
}
