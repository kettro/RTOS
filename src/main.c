/*
 * main.c
 */
#include "mgmt.h"
#include "process.h"
#include "syslib.h"
#include "systicklib.h"
#include "uartlib.h"
#include "alloc.h"
#include "wm.h"
#include "threads.h"
#include "msg.h"
#include "spilib.h"
#include "gpiof.h"

void init(void);

int main(void) {
  intDisable();
  init();
  createThreads();
  intEnable();
  SVC();
  while(1);
}

void init(void)
{
  SYSinit();
  SYSTICK_init();
  UART0_init();
  alloc_init();
  msg_init();
  //wm_init();
  GPIOF_init();
  spi_init();
  mgmt_init();
}
