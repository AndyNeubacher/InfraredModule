/*
 * appl.c
 *
 * Created: 25.04.2012 19:48:52
 *  Author: neubi
 */


#include "../system/main.h"
#include "../system/tools.h"
#include "../irmp/irmp.h"
#include "../irmp/irsnd.h"
#include "../uart/uart_api.h"
#include "appl.h"
#include "keyboard.h"
#include "version.h"



/**
*  main application -> should loop endless, handle event, ...
*
* @ param  none
* @ return none
*/
void Application(void)
{
    // send welcome-msg to host-pc
    UartSendMessage((t_uart_tx_msg*)SYSTEM_STARTUP_WELCOME_STRING, (sizeof(SYSTEM_STARTUP_WELCOME_STRING)-2));
    
    
    while(1)
    {
        if(IsEventPending(EVENT_10MS_TIMER))
        {
            ClearEvent(EVENT_10MS_TIMER);
        }

        if(IsEventPending(EVENT_UART_RX_MSG))
        {
            ClearEvent(EVENT_UART_RX_MSG);
            ApplUartMsgReceivedHandler();
        }


        if(IsEventPending(EVENT_UART_TX_MSG))
        {
            ClearEvent(EVENT_UART_TX_MSG);
            UartTransmitMessageQueue();
        }


        if(IsEventPending(EVENT_IR_CMD_RECEIVED))
        {
            ClearEvent(EVENT_IR_CMD_RECEIVED);
            ApplIrMsgReceivedHandler();
        }
    }
}


/**
*  get received IR data (from infrared interface) and process
*
* @ param  none
* @ return TRUE if routing to UART was successful
*/
void ApplIrMsgReceivedHandler(void)
{
    //BYTE key;
	IRMP_DATA irmp_data;


    irmp_get_data(&irmp_data);                              // get IR msg outof IRMP-stack
    SendIrCmdToPc(&irmp_data);
    
    //if(isKeyboardCmd(irmp_data, &key) == TRUE)
        //SendKeyboardCmdToPc(key);
    //else
        //SendIrCmdToPc(irmp_data);
}


/**
*  send keyboard-character to PC -> emulate keyboard
*
* @ param  keyboard key
* @ return TRUE if cmd was correct
*/
//void SendKeyboardCmdToPc(BYTE key)
//{
    //if ((key >= 0x20 && key < 0x7F) || key >= 0xA0)     // show only printable characters
        //UartTransmitSingleByte(key);
    //else                                                // it's a non-printable key
        //UartTransmitSingleByte(key);
//}


/**
*  send any received cmd to PC
*
* @ param  IRMP_DATA struct
* @ return TRUE if cmd was sent correctly
*/
BYTE SendIrCmdToPc(IRMP_DATA* ir)
{
    t_uart_tx_msg msg;

    msg.type = UART_TYPE_IR_RECEIVED_DATA;                  // create UART msg and send to PC
    msg.data[0] = 'p';
    Byte2AsciiHex(ir->protocol, &msg.data[1]);
    msg.data[3] = 'a';
    Word2AsciiHex(ir->address, &msg.data[4]);
    msg.data[8] = 'c';
    Word2AsciiHex(ir->command, &msg.data[9]);
    msg.data[13] = 'f';
    Byte2AsciiHex(ir->flags, &msg.data[14]);

    return UartSendMessage(&msg, 16);
}


/**
*  get received UART data and process
*
* @ param  none
* @ return TRUE if cmd was correct
*/
void ApplUartMsgReceivedHandler(void)
{
    IRMP_DATA ir;
    t_uart_rx_msg rx_msg;
    union union_w_b val;

    while(UartReceiveMessage(&rx_msg))             // loop till msg queue is empty
    {
		switch(rx_msg.type)
		{
			case UART_TYPE_IR_SEND_DATA:
                    Ascii2Hex(&rx_msg.data[1], ((BYTE*)(&val)), sizeof(BYTE));       // 'p'
                    ir.protocol = val.b[0];
                    Ascii2Hex(&rx_msg.data[4], ((BYTE*)(&val)), sizeof(WORD));       // 'a'
                    ir.address = BSWAP_16(val.w);
                    Ascii2Hex(&rx_msg.data[9], ((BYTE*)(&val)), sizeof(WORD));       // 'c'
                    ir.command = BSWAP_16(val.w);
                    Ascii2Hex(&rx_msg.data[14], ((BYTE*)(&val)), sizeof(BYTE));      // 'f'
                    ir.flags = val.b[0];

		            irsnd_send_data(&ir, TRUE);    // send cmd via IR interface
				break;

			case UART_TYPE_SYSTEM_RESET:
                    soft_reset();
				break;
                
            case UART_TYPE_SYSTEM_VERSION:
                    SendSystemVersion();
                break;
				
			default:
			    break;
		}
    }
}


/**
*  send systemversion to PC
*
* @ param  none
* @ return none
*/
void SendSystemVersion(void)
{
    BYTE len = 0;
    t_uart_tx_msg msg;


    msg.type = UART_TYPE_SYSTEM_VERSION;                  // create UART msg and send to PC
    
    memcpy(&msg.data[0], SYSTEM_VERSION_STRING, sizeof(SYSTEM_VERSION_STRING));
    len += sizeof(SYSTEM_VERSION_STRING);

    msg.data[len-1] = '-';
    len += 1;

    memcpy(&msg.data[len-1], SYSTEM_VERSION_DATE, sizeof(SYSTEM_VERSION_DATE));
    len += sizeof(SYSTEM_VERSION_DATE);
    
    
    UartSendMessage(&msg, len);
}