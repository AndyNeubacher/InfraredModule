/*
 * InfraredModule.h
 *
 * Created: 25.04.2012 15:30:45
 *  Author: neubi
 */


#ifndef INFRAREDMODULE_H_
#define INFRAREDMODULE_H_


#define F_CPU                           12000000

#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <string.h>


#define SYSTEM_EMULATOR_DEBUGGING       0
#ifdef  SYSTEM_EMULATOR_DEBUGGING
#define ASSERT(x)		                if(x){__asm__ __volatile__ ("break");}
#else
#define ASSERT(x)
#endif


typedef signed char                     SBYTE;
typedef unsigned char                   BYTE;
typedef signed int                      SWORD;
typedef unsigned int                    WORD;
typedef unsigned long                   LONG;

union union_w_b
{
    WORD w;
    BYTE b[2];
};



#define TRUE                            1
#define FALSE                           0

#define EVENT_IDLE                      0x00
#define EVENT_10MS_TIMER                0x01
#define EVENT_UART_RX_MSG               0x02
#define EVENT_UART_TX_MSG               0x04
#define EVENT_UNUSED_1                  0x08
#define EVENT_UNUSED_2                  0x10
#define EVENT_UNUSED_3                  0x20
#define EVENT_UNUSED_4                  0x40
#define EVENT_IR_CMD_RECEIVED           0x80

#define EVENT_RESULT_NOT_IMPLEMENTED    0x00
#define EVENT_RESULT_TIMEOUT            0x01
#define EVENT_RESULT_MSG                0x02
#define EVENT_RESULT_SUCCESS            0x04


#define GetMutex()                      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#define ReleaseMutex()                  }
#define MutexFunc(func)                 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {func}

#define SetEvent(event)                 MutexFunc(System.Event.Flags |= (event);)
#define SetEventMsg(event, msg)         MutexFunc(System.Event.Flags |= (event); System.Event.Msg = (msg);)
#define ClearEvent(event)               MutexFunc(System.Event.Flags &= ~(event);)
#define IsEventPending(event)           (System.Event.Flags & (event))

#define soft_reset()                    do{wdt_enable(WDTO_15MS); for(;;) {} } while(0)



typedef struct
{
    BYTE inc_50us;
    BYTE inc_10ms;
    LONG inc_1sec;
} t_time;

typedef struct
{
    volatile BYTE Flags;
    BYTE Msg;
} t_event;


typedef struct
{
    t_time      Time;
    t_event     Event;
} t_system;




extern t_system   System;



void Timer1Init(void);

BYTE UartInit(BYTE baudrate);

void SystemInit(void);

BYTE WaitEventTimeout(BYTE event, WORD timeout);

void Sleep(WORD time);


#endif /* INFRAREDMODULE_H_ */
