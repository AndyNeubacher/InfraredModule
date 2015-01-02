/*
 * uart.h
 *
 * Created: 26.04.2012 15:42:17
 *  Author: neubi
 */ 


#ifndef UART_H_
#define UART_H_



#define UART_BUF_STATUS_EMPTY       0x00
#define UART_BUF_STATUS_FILLING     0x01
#define UART_BUF_STATUS_VALID       0x02

#define UART_TX_EN                  0x08
#define UART_RX_EN                  0x10
#define UART_RX_INT_EN              0x80
#define UART_URSEL                  0x80
#define UART_8BIT                   0x06

#define UartEnableInterrupt()       (UCSRB |= UART_RX_INT_EN)
#define UartDisableInterrupt()      (UCSRB &= ~UART_RX_INT_EN)


/* strctures for RECEIVE */
typedef struct
{
   BYTE                 data[sizeof(t_uart_rx_msg)];
   BYTE                 status;
   BYTE                 len;
   BYTE                 id;
} t_serial_rx_buf;

typedef struct  
{
    t_serial_rx_buf     Msg[UART_RX_MSG_BUFFER_NUM];
    t_serial_rx_buf*    pActBuf;
    BYTE                nextIdForQueue;
    BYTE                nextIdForReceive;
} t_serial_rx;


/* structures for TRANSMIT */
typedef struct
{
    BYTE                data[sizeof(t_uart_tx_msg)];
    BYTE                status;
    BYTE                len;
    BYTE                id;
} t_serial_tx_buf;

typedef struct
{
    t_serial_tx_buf     Msg[UART_TX_MSG_BUFFER_NUM];
    BYTE                nextIdForQueue;
    BYTE                nextIdForSend;
} t_serial_tx;


/* main UART struct */
typedef struct
{
    t_serial_rx         Rx;
    t_serial_tx         Tx;
} t_serial;

extern t_serial Uart;





t_serial_rx_buf* _uart_rx_get_free_buffer(void);

t_serial_rx_buf* _uart_rx_get_next_msg(void);

void _uart_rx_release_buffer(t_serial_rx_buf* ptr);


t_serial_tx_buf* _uart_tx_get_free_buffer(void);

void _uart_tx_release_buffer(t_serial_tx_buf* ptr);



#endif /* UART_H_ */
