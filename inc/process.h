#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdint.h>
#include "data_structures.h"
#include "syslib.h"

#define returnToMSP        0xFFFFFFF9
#define returnToPSP        0xFFFFFFFD

#define STACKSIZE          1024 // size 3
#define STACKFRAMESIZE     32
#define HALFSFSIZE         16

uint32_t getPSP(void);
uint32_t getMSP(void);
void setPSP(volatile uint32_t newPSP);
void setMSP(volatile uint32_t newMSP);
uint32_t getSP(void);
void setMgmtArg(volatile uint32_t arg);

#define saveRegs()         __asm(" mrs r0,psp");\
                           __asm(" stmdb r0!, {r4-r11}");\
                           __asm(" msr psp,r0");

#define restoreRegs()      __asm(" mrs r0,psp");\
                           __asm(" ldmia r0!,{r4-r11}");\
                           __asm(" msr psp,r0");

#endif
