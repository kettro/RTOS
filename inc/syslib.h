/* File: syslib.h
 * Author: Ross Kettleson
 * Revision Oct 24 2016
 */
#ifndef __SYSLIB_H__
#define __SYSLIB_H__

#include <stdint.h>
#include "data_structures.h"

#define SVC()           __asm(" SVC #0")
#define intEnable()     __asm(" cpsie i")
#define intDisable()    __asm(" cpsid i")

void SYSinit(void);

// Defines
#define NVIC_ST_CTRL_R          (reg_t(0xE000E010))  // SysTick Control and Status Register (STCTRL)
#define NVIC_ST_RELOAD_R        (reg_t(0xE000E014))  // Systick Reload Value Register (STRELOAD)
#define NVIC_ST_CTRL_COUNT      0x00010000  // Count Flag for STCTRL
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source for STCTRL
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt Enable for STCTRL
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Enable for STCTRL
#define NVIC_INT_CTRL_R         (reg_t(0xE000ED04))
#define TRIGGER_PENDSV          (0x10000000)
#define NVIC_SYS_PRI3_R         (reg_t(0xE000ED20))
#define PENDSV_BOT_PRI          (0x00E00000)

struct SYSCTL_RCGC1    // Run Mode Clock Gating Control Register 1 bits
{
  volatile unsigned UART0 : 1;   // UART0 Clock Gating Control
  volatile unsigned UART1 : 1;   // UART1 Clock Gating Control
  volatile unsigned UART2 : 1;   // UART2 Clock Gating Control
  const unsigned RES3 : 1;      // Reserved
  volatile unsigned SSI0 : 1;   // SSI0 Clock Gating Control
  volatile unsigned SSI1 : 1;   // SSI1 Clock Gating Control
  const unsigned RES6_7 : 2;      // Reserved
  volatile unsigned QEI0 : 1;   // QEI0 Clock Gating Control
  volatile unsigned QEI1 : 1;   // QEI1 Clock Gating Control
  const unsigned RES10_11 : 2;    // Reserved
  volatile unsigned I2C0 : 1;   // I2C0 Clock Gating Control
  const unsigned RES13 : 1;      // Reserved
  volatile unsigned I2C1 : 1;   // I2C1 Clock Gating Control
  const unsigned RES15 : 1;      // Reserved
  volatile unsigned TIMER0 : 1;   // TIMER0 Clock Gating Control
  volatile unsigned TIMER1 : 1;   // TIMER1 Clock Gating Control
  volatile unsigned TIMER2 : 1;   // TIMER2 Clock Gating Control
  volatile unsigned TIMER3 : 1;   // TIMER3 Clock Gating Control
  const unsigned RES20_23 : 4;      // Reserved
  volatile unsigned COMP0 : 1;  // Analog Comparator 0 Clock Gating
  volatile unsigned COMP1 : 1;  // Analog Comparator 1 Clock Gating
  volatile unsigned COMP2 : 1;  // Analog Comparator 2 Clock Gating
  const unsigned RES27 : 1;         // Reserved
  volatile unsigned I2S0 : 1;   // I2S0 Clock Gating
  const unsigned RES29 : 1;         // Reserved
  volatile unsigned EPI0: 1;     // EPI0 Clock Gating
  const unsigned RES31 : 1;         // Reserved
};

struct SYSCTL_RCGC2    // Run Mode Clock Gating Control Register 2 bits
{
  volatile unsigned GPIOA : 1;   // PORT A Clock Gating Control
  volatile unsigned GPIOB : 1;   // PORT B Clock Gating Control
  volatile unsigned GPIOC : 1;   // PORT C Clock Gating Control
  volatile unsigned GPIOD : 1;   // PORT D Clock Gating Control
  volatile unsigned GPIOE : 1;   // PORT E Clock Gating Control
  volatile unsigned GPIOF : 1;   // PORT F Clock Gating Control
  volatile unsigned GPIOG : 1;   // PORT G Clock Gating Control
  volatile unsigned GPIOH : 1;   // PORT H Clock Gating Control
  volatile unsigned GPIOJ : 1;   // PORT J Clock Gating Control
  const unsigned RES9_12 : 4;     // Reserved
  volatile unsigned UDMA : 1;   // Micro-DMA Clock Gating Control
  const unsigned RES14_15 : 2;     // Reserved
  volatile unsigned USB0 : 1;   // USB0 Clock Gating Control
  const unsigned RES17_27 : 11;    // Reserved
  volatile unsigned EMAC0 : 1;   // MAC0 Clock Gating Control
  const unsigned RES29 : 1;      // Reserved
  volatile unsigned EPHY0 : 1;   // PHY0 Clock Gating Control
  const unsigned RES31 : 1;      // Reserved
};
extern struct SYSCTL_RCGC1 * const SYSCTL_RCGC1_R; //= (struct SYSCTL_RCGC1 *) 0x400FE104;
extern struct SYSCTL_RCGC2 * const SYSCTL_RCGC2_R; // = (struct SYSCTL_RCGC2 *) 0x400FE108;


#define systickDisable()        (NVIC_ST_CTRL_R &= ~(NVIC_ST_CTRL_ENABLE))
#define systickEnable()         (NVIC_ST_CTRL_R |= (NVIC_ST_CTRL_ENABLE))
#define triggerPendSV()         (NVIC_INT_CTRL_R |= TRIGGER_PENDSV)

#endif

