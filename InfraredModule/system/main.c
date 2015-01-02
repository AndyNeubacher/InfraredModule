/*
 * @file main.c
 *
 * @brief System startup and low-level initialization
 *
 * @dat 25.04.2012 15:12:33
 * @author Andreas Neubacher
 */


#include "main.h"
#include "../irmp/irmp.h"
#include "../irmp/irsnd.h"
#include "../uart/uart_api.h"
#include "../appl/appl.h"


t_system   System;




/**
*  timer 1 init -> setup output-compare to 50µs int
*
* @param  none
* @return none
*/
void Timer1Init(void)
{
    OCR1A   =  (F_CPU / F_INTERRUPTS) - 1;      // compare value: 1/20000 of CPU frequency
    TCCR1B  = (1 << WGM12) | (1 << CS10);       // switch CTC Mode on, set prescaler to 1
    TIMSK   = 1 << OCIE1A;                      // OCIE1A: Interrupt by timer compare
}


/**
*  init internal UART with 9600 or 115200 baud
*
* @param  baudrate
* @return TRUE on succcess, FALSE on invalid param
*/
BYTE UartInit(BYTE baudrate)
{
    if((baudrate != UART_BAUDRATE_115200) && (baudrate != UART_BAUDRATE_9600))
        return FALSE;

    UCSRB = 0x00;                                   // disable uart

    if(baudrate == UART_BAUDRATE_9600)
        UBRRL = 155;                                // 9600 baud
    else
        UBRRL = 12;                                 // 115200 baud

    UBRRH = 0x00;
    UCSRA |= (1<<U2X);                              // enable fast mode to achieve the baudrates
    UCSRB = ((1<<RXCIE) | (1<<RXEN)  | (1<<TXEN));
    UCSRC = ((1<<URSEL) | (1<<UCSZ0) | (1<<UCSZ1));

    UartInitDataBuffer(TRUE, TRUE);

    return TRUE;
}


/**
*  init system variables
*
* @param  none
* @return none
*/
void SystemInit(void)
{
    System.Time.inc_50us = 0;
    System.Time.inc_10ms = 0;
    System.Time.inc_1sec = 0;

    System.Event.Flags   = 0;
    System.Event.Msg     = 0;
}


/**
*  timer1 interrupt service routine, called 20000 times per second
*
* @param  none
* @return none
*/
ISR(TIMER1_COMPA_vect)
{
    // handle infrared-timer
    if(!irsnd_ISR())         // call irsnd ISR
      irmp_ISR();            // call irmp ISR


    // update system-clock
    if(System.Time.inc_50us++ > 199)
    {
        SetEvent(EVENT_10MS_TIMER);
        System.Time.inc_50us = 0;
        if(System.Time.inc_10ms++ > 99)
        {
            System.Time.inc_10ms = 0;
            System.Time.inc_1sec++;
        }
    }
}


/**
*  wait for given event with a given timeout
*
* @param  type of event
* @ param  number of 10*milliseconds
* @ return EVENT_RESULT_SUCCESS if param-event occoured, EVENT_RESULT_TIMEOUT if timeout exceeded
*/
BYTE WaitEventTimeout (BYTE event, WORD timeout)
{
    while(timeout--)
    {
        ClearEvent(EVENT_10MS_TIMER);
        while(!(System.Event.Flags & (EVENT_10MS_TIMER | event)));

        if(IsEventPending(event))      // if given event received -> return success
            return EVENT_RESULT_SUCCESS;
    }

    return EVENT_RESULT_TIMEOUT;
}


/**
*  sleep for given time*10 milliseconds
*
* @param  number of 10*milliseconds
* @ return none
*/
void Sleep(WORD time)
{
    ClearEvent(EVENT_10MS_TIMER);
    while(time-- > 0)
    {
        while(!(System.Event.Flags & EVENT_10MS_TIMER));        
        ClearEvent(EVENT_10MS_TIMER);
    }
}


/**
*  main program entry -> won't return
*
* @ param  none
* @ return none
*/
int main (void)
{
    SystemInit();                       // init variables and portpins
    irmp_init();                        // initialize irmp
    irsnd_init();                       // initialize irsnd
    Timer1Init();                       // initialize timer
    UartInit(UART_BAUDRATE_115200);     // init UART
    sei();                              // enable interrupts

    Application();                      // start main application (loops endless)

    return 1;
}
