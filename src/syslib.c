/* File: SysLib.c
 * Author: Ross Kettleson
 * Revision Oct 16 2016
 */

#include "syslib.h"

// Clock Configuration Register
#define SYSCTRL_RCC_R           (reg_t(0x400FE060))

#define CLEAR_USRSYSDIV         0xF83FFFFF	// Clear USRSYSDIV Bits
#define SET_BYPASS              0x00000800	// Set BYPASS Bit

struct SYSCTL_RCGC1 * const SYSCTL_RCGC1_R = (struct SYSCTL_RCGC1 *) 0x400FE104;
struct SYSCTL_RCGC2 * const SYSCTL_RCGC2_R  = (struct SYSCTL_RCGC2 *) 0x400FE108;


/* Function: interruptEnable
 * Parameters: 
 * Returns: 
 * Description: Enables global interrupts
 */
void interruptEnable(void)
{
  __asm(" cpsie i");
}

/* Function: interruptDisable
 * Parameters: 
 * Returns: 
 * Description: Disables global interrupts
 */
void interruptDisable(void)
{
  __asm(" cpsid i");
}

/* Function: SYSinit
 * Parameters: 
 * Returns: 
 * Description: initializes system settings
 */
void SYSinit(void)
{
  // Set up PIOSC
  SYSCTRL_RCC_R = (SYSCTRL_RCC_R & CLEAR_USRSYSDIV) | SET_BYPASS;

}
