#ifndef MAIN_H
#define MAIN_H
#include <stm32f10x_lib.h>
#include "system_init.h"
#include "OUT.H"
#include "CAN.H"
#include "ADC.H"

#define CANMsgLen  40

struct CANRxList
{
    CanRxMsg RxMsg;
    struct CANRxList *next;
} ;

extern struct CANRxList CANMsg[CANMsgLen];
extern struct CANRxList *Head;
extern struct CANRxList *Current;




#endif

