/* File: SystickLib.h
 * Author: Ross Kettleson
 * Revision Oct 16 2016
 */

#include "systicklib.h"
#include "syslib.h"
#include "process.h"
#include "mgmt.h"
#include "supervisor.h"
#include <stdint.h>

extern pcb_t* current_process;
extern pcb_t* ProcessQHead[TOTAL_PLEVELS];

/* Function: SYSTICK_ISR
 * Parameters: 
 * Returns: 
 * Description: Interrupt Service Routine for the Systick
 *              Triggers PENDSV
 */
void SYSTICK_ISR(void)
{
  triggerPendSV();
}

/* Function: SYSTICK_init
 * Parameters: 
 * Returns: 
 * Description: initializes the systick.
 */
void SYSTICK_init(void)
{
  // set tick period for 0.1s
  NVIC_ST_RELOAD_R = 1604150; // give or take, as per Nick, who tested it for 10 minutes
  // enable systick interrupts
  NVIC_ST_CTRL_R |= NVIC_ST_CTRL_INTEN;
  // set clock as internal
  NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_ENABLE;
  // exit
}
