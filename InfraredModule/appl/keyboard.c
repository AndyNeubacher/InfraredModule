/*
 * keyboard.c
 *
 * Created: 04.01.2013 19:43:42
 *  Author: neubi
 */ 

#include "../system/main.h"
#include "../irmp/irmp.h"
#include "keyboard.h"



/**
*  check if received ir-cmd comes from an infrared keyboard
*
* @ param  IRMP data
* @ return TRUE if keyboard command
*/
BYTE isKeyboardCmd(IRMP_DATA ir, BYTE* key)
{
    if((ir.protocol == IRMP_FDC_PROTOCOL) && (*key = get_fdc_key(ir.command)) != 0)
        return TRUE;

    return FALSE;
}


/**
*  decode infrared cmd to correct keyboard-key
*
* @ param  cmd
* @ return keyboard-key
*/
BYTE get_fdc_key(WORD cmd)
{
    static BYTE key_table[128] =
    {
        // 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
        0,  '^', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'ß', '´',  0,  '\b',
        '\t','q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'ü', '+',  0,   0,  'a',
        's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ö', 'ä', '#',  '\r', 0,  '<', 'y', 'x',
        'c', 'v', 'b', 'n', 'm', ',', '.', '-',  0,   0,   0,   0,   0,  ' ',  0,   0,
        
        0,  '°', '!', '"', '§', '$', '%', '&', '/', '(', ')', '=', '?', '`',  0,  '\b',
        '\t','Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 'Ü', '*',  0,   0,  'A',
        'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ö', 'Ä', '\'','\r', 0,  '>', 'Y', 'X',
        'C', 'V', 'B', 'N', 'M', ';', ':', '_',  0,   0,   0,   0,   0,  ' ',  0,   0
    };
    static BYTE state;
    
    BYTE key = 0;
    
    switch (cmd)
    {
        case 0x002C: state |=  STATE_LEFT_SHIFT;    break;              // pressed left shift
        case 0x00AC: state &= ~STATE_LEFT_SHIFT;    break;              // released left shift
        case 0x0039: state |=  STATE_RIGHT_SHIFT;   break;              // pressed right shift
        case 0x00B9: state &= ~STATE_RIGHT_SHIFT;   break;              // released right shift
        case 0x003A: state |=  STATE_LEFT_CTRL;     break;              // pressed left ctrl
        case 0x00BA: state &= ~STATE_LEFT_CTRL;     break;              // released left ctrl
        case 0x003C: state |=  STATE_LEFT_ALT;      break;              // pressed left alt
        case 0x00BC: state &= ~STATE_LEFT_ALT;      break;              // released left alt
        case 0x003E: state |=  STATE_RIGHT_ALT;     break;              // pressed left alt
        case 0x00BE: state &= ~STATE_RIGHT_ALT;     break;              // released left alt
        
        case 0x006e: key = KEY_ESCAPE;              break;
        case 0x004b: key = KEY_INSERT;              break;
        case 0x004c: key = KEY_DELETE;              break;
        case 0x004f: key = KEY_LEFT;                break;
        case 0x0050: key = KEY_HOME;                break;
        case 0x0051: key = KEY_END;                 break;
        case 0x0053: key = KEY_UP;                  break;
        case 0x0054: key = KEY_DOWN;                break;
        case 0x0055: key = KEY_PAGE_UP;             break;
        case 0x0056: key = KEY_PAGE_DOWN;           break;
        case 0x0059: key = KEY_RIGHT;               break;
        case 0x0400: key = KEY_MOUSE_1;             break;
        case 0x0800: key = KEY_MOUSE_2;             break;
        
        default:
        {
            if (!(cmd & 0x80))                      // pressed key
            {
                if (cmd >= 0x70 && cmd <= 0x7F)     // function keys
                    key = cmd + 0x10;               // 7x -> 8x
                else if (cmd < 64)                  // key listed in key_table
                {
                    if (state & (STATE_LEFT_ALT | STATE_RIGHT_ALT))
                    {
                        switch (cmd)
                        {
                            case 0x0003: key = '²';     break;
                            case 0x0008: key = '{';     break;
                            case 0x0009: key = '[';     break;
                            case 0x000A: key = ']';     break;
                            case 0x000B: key = '}';     break;
                            case 0x000C: key = '\\';    break;
                            case 0x001C: key = '~';     break;
                            case 0x002D: key = '|';     break;
                            case 0x0034: key = 'µ';     break;
                        }                            
                    }
                }
                else if (state & (STATE_LEFT_CTRL))
                {
                    if (key_table[cmd] >= 'a' && key_table[cmd] <= 'z')
                        key = key_table[cmd] - 'a' + 1;
                    else
                        key = key_table[cmd];
                }
                else
                {
                    int idx = cmd + ((state & (STATE_LEFT_SHIFT | STATE_RIGHT_SHIFT)) ? 64 : 0);
                    
                    if (key_table[idx])
                        key = key_table[idx];
                }
            }
        }
        break;
    }


    return key;
}
    