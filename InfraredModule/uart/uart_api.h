/*
 * uart_api.h
 *
 * Created: 04.05.2012 18:18:47
 *  Author: neubi
 */


#ifndef UART_API_H_
#define UART_API_H_


/* defines for protocol */
#define UART_SOT                    0x02
#define UART_EOT                    0x03


/* defines for send-receive buffer sizes */
#define UART_TX_MSG_BUFFER_NUM      4
#define UART_TX_MSG_BUFFER_LEN      20
#define UART_RX_MSG_BUFFER_NUM      4
#define UART_RX_MSG_BUFFER_LEN      20


#define UART_BAUDRATE_9600          1
#define UART_BAUDRATE_115200        2



// struct for 'user-rx-message'
typedef struct
{
    BYTE type;
    BYTE data[UART_RX_MSG_BUFFER_LEN];
} t_uart_rx_msg;


// struct for 'user-tx-message'
typedef struct
{
    BYTE type;
    BYTE data[UART_TX_MSG_BUFFER_LEN];
} t_uart_tx_msg;





void UartInitDataBuffer(BYTE initRx, BYTE initTx);

void UartTransmitMessageQueue(void);

BYTE UartSendMessage(t_uart_tx_msg* msg, BYTE length);

BYTE UartReceiveMessage(t_uart_rx_msg* new_msg);

void UartTransmitSingleByte(BYTE data);


#endif /* UART_API_H_ */
