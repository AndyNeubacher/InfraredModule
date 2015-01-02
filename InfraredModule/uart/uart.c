/*
 * uart.c
 *
 * Created: 26.04.2012 15:42:03
 *  Author: neubi
 */


#include "../system/main.h"
#include "uart_api.h"
#include "uart.h"


t_serial Uart;


/*!
 *  flush UART buffers (Rx and Tx)
 *
 * @ param  initRx = TRUE clears the receive-buffer-structure, FALSE = do nothing
 * @ param  initTx = TRUE clears the receive-buffer-structure, FALSE = do nothing
 * @ return none
 */
void UartInitDataBuffer(BYTE initRx, BYTE initTx)
{
    BYTE i;

    GetMutex();
    {
        if(initRx == TRUE)
        {
            Uart.Rx.nextIdForQueue   = 0;
            Uart.Rx.nextIdForReceive = 0;
            Uart.Rx.pActBuf          = &Uart.Rx.Msg[0];
            for(i=0; i<UART_RX_MSG_BUFFER_NUM; i++)
                _uart_rx_release_buffer(&Uart.Rx.Msg[i]);
        }

        if(initTx == TRUE)
        {
            Uart.Tx.nextIdForQueue   = 0;
            Uart.Tx.nextIdForSend    = 0;
            for(i=0; i<UART_TX_MSG_BUFFER_NUM; i++)
                _uart_tx_release_buffer(&Uart.Tx.Msg[i]);
        }
    }
    ReleaseMutex();
}


/*!
*  UART receive interrupt -> handle received databytes
*
* @ param  none
* @ return none
*/
ISR(USART_RXC_vect)
{
    BYTE error=0;
    BYTE data=0;

    error = UCSRA;
    data = UDR;

    if(UCSRA & 0x18)                        // check overrun or frameerror ?
        while(UCSRA & 0x80) error = UDR;    // flush databuffer
    else
    {
        switch(data)
        {
            // start-of-text received
            case UART_SOT:
                    // if we get a SOT without an EOT from the previous msg
                    if((Uart.Rx.pActBuf != NULL) &&
                       (Uart.Rx.pActBuf->status == UART_BUF_STATUS_FILLING))
                        Uart.Rx.pActBuf->len = 0;       // 'flush' old data and re-start from idx=0
                    else
                        _uart_rx_get_free_buffer();
                break;

            // end-of-text received
            case UART_EOT:
                    if(Uart.Rx.pActBuf->status == UART_BUF_STATUS_FILLING)
                    {
                        if(Uart.Rx.pActBuf->len != 0)   // if we get many EOT in a row ...
                        {
                            Uart.Rx.pActBuf->status = UART_BUF_STATUS_VALID;
                            SetEvent(EVENT_UART_RX_MSG);
                        }
                        else
                            Uart.Rx.pActBuf->len = 0;   // 'flush' old data and re-start from idx=0
                    }
                break;

            // save msg-data to buffer
            default:
                    if((Uart.Rx.pActBuf->status == UART_BUF_STATUS_FILLING) &&
                       (Uart.Rx.pActBuf->len < sizeof(Uart.Rx.pActBuf->data)) &&
                       (Uart.Rx.pActBuf != NULL))
                            Uart.Rx.pActBuf->data[Uart.Rx.pActBuf->len++] = data;     // safe data
                break;
        }
    }
}


/**
*  search for an empty UART-Rx buffer; this sets the 'Uart.pActRxBuf'
*
* @ param  none
* @ return ptr to RxBuf structure
*/
t_serial_rx_buf* _uart_rx_get_free_buffer(void)
{
    BYTE i;

    // get an empty buffer
    for(i=0; i<UART_RX_MSG_BUFFER_NUM; i++)
    {
        if(Uart.Rx.Msg[i].status == UART_BUF_STATUS_EMPTY)
        {
            Uart.Rx.pActBuf         = &Uart.Rx.Msg[i];
            Uart.Rx.pActBuf->status = UART_BUF_STATUS_FILLING;
            Uart.Rx.pActBuf->id     = Uart.Rx.nextIdForReceive++;
            return Uart.Rx.pActBuf;
        }
    }

    Uart.Rx.pActBuf = NULL;
    return NULL;                // no empty buffer found
}


/**
*  get next received UART message from RxMessage Queue
*
* @ param  none
* @ return ptr to a filled UartRx message-structure
*/
t_serial_rx_buf* _uart_rx_get_next_msg(void)
{
    BYTE i;

    for(i=0; i<UART_RX_MSG_BUFFER_NUM; i++)
    {
        if((Uart.Rx.nextIdForQueue == Uart.Rx.Msg[i].id) &&
		   (Uart.Rx.Msg[i].status == UART_BUF_STATUS_VALID))
        {
            Uart.Rx.nextIdForQueue++;
            return &Uart.Rx.Msg[i];
        }
    }

    return NULL;
}


/**
*  free given buffer and clean data if requested
*
* @ param  ptr to UART-Rx buffer
* @ return none
*/
void _uart_rx_release_buffer(t_serial_rx_buf* ptr)
{
    ptr->id     = 0;
    ptr->len    = 0;
    ptr->status = UART_BUF_STATUS_EMPTY;

	memset(ptr->data, 0x00, sizeof(ptr->data));
}


/**
*  get an empty UART-Tx Buffer
*
* @ param  none
* @ return ptr to an empty UartTx structure
*/
t_serial_tx_buf* _uart_tx_get_free_buffer(void)
{
    BYTE i;

    for(i=0; i<UART_TX_MSG_BUFFER_NUM; i++)
    {
        if(Uart.Tx.Msg[i].status == UART_BUF_STATUS_EMPTY)
        {
            Uart.Tx.Msg[i].status = UART_BUF_STATUS_FILLING;
            Uart.Tx.Msg[i].id     = Uart.Tx.nextIdForQueue++;;
            return &Uart.Tx.Msg[i];
        }
    }

    return NULL;            // no empty buffer found
}


/**
*  send message via UART
*
* @ param  ptr to payload-data
* @ return TRUE on success, FALSE if failed (no empty buffer found)
*/
BYTE UartSendMessage(t_uart_tx_msg* msg, BYTE length)
{
    t_serial_tx_buf* buf;
    BYTE len = (sizeof(t_uart_tx_msg) - sizeof(msg->data)) + length;  // sizeof header + msglen


    if(len > sizeof(t_serial_tx_buf))               // payload fits in buffer?
        return FALSE;

    buf = _uart_tx_get_free_buffer();               // get empty buffer
    if(buf == NULL)
        return FALSE;                               // no valid buffer found

    memcpy((BYTE*)&buf->data, (BYTE*)msg, len);     // copy msg to tx-buffer

    buf->len     = len;
    buf->status  = UART_BUF_STATUS_VALID;           // mark buffer has filled up and ready to transmit

    SetEvent(EVENT_UART_TX_MSG);                    // set event to trigger transmit
    return TRUE;
}


void UartTransmitSingleByte(BYTE data)
{
    while ((UCSRA & 0x20) == 0x00);
    UDR = data;                 // write data
}


/**
*  transmit next pending message
*
* @ param  none
* @ return none
*/
void UartTransmitMessageQueue(void)
{
    BYTE i;
    t_serial_tx_buf* buf;


    // loop till all pending messages are sent
    while(Uart.Tx.nextIdForQueue != Uart.Tx.nextIdForSend)
    {
        buf = NULL;

        // search next message in queue
        for(i=0; i<UART_TX_MSG_BUFFER_NUM; i++)
        {
            if((Uart.Tx.nextIdForSend == Uart.Tx.Msg[i].id) &&
			   (Uart.Tx.Msg[i].status == UART_BUF_STATUS_VALID))
            {
                buf = &Uart.Tx.Msg[i];
                break;
            }
        }

        if(buf == NULL)                              // not found -> should never happen? ... otherwise bugfix me!
            return UartInitDataBuffer(FALSE, TRUE);

        UartTransmitSingleByte(UART_SOT);               // transmit message
        for(i=0; i<buf->len; i++)
            UartTransmitSingleByte(buf->data[i]);
        UartTransmitSingleByte(UART_EOT);

        _uart_tx_release_buffer(buf);                // free msg-buffer

        Uart.Tx.nextIdForSend++;                     // increment index for next msg in queue
    }
}


/**
*  free given buffer and clean data if requested
*
* @ param  ptr to UART-Tx buffer
* @ return none
*/
void _uart_tx_release_buffer(t_serial_tx_buf* ptr)
{
    ptr->id     = 0;
    ptr->len    = 0;
    ptr->status = UART_BUF_STATUS_EMPTY;

    memset(ptr->data, 0x00, sizeof(ptr->data));
}


/**
*  get next user-message from UART buffer
*
* @ param  ptr where next msg should be copied
* @ return TRUE if found pending message, FALSE if no new message found
*/
BYTE UartReceiveMessage(t_uart_rx_msg* msg)
{
    t_serial_rx_buf* buf_ptr;


    buf_ptr = _uart_rx_get_next_msg();
    if(buf_ptr == NULL)
        return FALSE;

    // copy content
    memcpy((BYTE*)msg, (BYTE*)buf_ptr->data, buf_ptr->len);
    _uart_rx_release_buffer(buf_ptr);

    return TRUE;
}
