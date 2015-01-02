/*
 * appl.h
 *
 * Created: 25.04.2012 19:49:08
 *  Author: neubi
 */ 


#ifndef APPL_H_
#define APPL_H_


// SYSTEM specific types
#define UART_TYPE_SYSTEM_RESET		'b'
#define UART_TYPE_SYSTEM_UPDATE     'u'
#define UART_TYPE_SYSTEM_VERSION    'v'

// UART-COMMUNICATION types
#define UART_TYPE_ACK               'a'
#define UART_TYPE_UNKOWN_CMD        'e'

// IR specific types
#define UART_TYPE_IR_SEND_DATA      's'
#define UART_TYPE_IR_RECEIVED_DATA  'r'





void Application(void);

void ApplIrMsgReceivedHandler(void);

//void SendKeyboardCmdToPc(BYTE key);

BYTE SendIrCmdToPc(IRMP_DATA* ir);

void ApplUartMsgReceivedHandler(void);

void SendSystemVersion(void);


#endif /* APPL_H_ */
