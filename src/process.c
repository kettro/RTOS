/* File: process.c
 * Author: Ross Kettleson
 * Revision Nov 14 2016
 * Purpose: Small Utility Functions for mgmt
 */

#include "process.h"

// Most of this, if not all lifted directly from Dr. Hughes'
// implementation from his website.

/* Name: getPSP
 * Parameters:
 * Returns: 32-bit integer memory location
 * Description: returns the value of the current PSP
 */
uint32_t getPSP(void)
{
  __asm(" mrs r0, psp");
  __asm(" bx lr");
  return 0;
}

/* Name: getMSP
 * Parameters:
 * Returns: 32-bit integer memory location
 * Description: returns the value of the current MSP
 */
uint32_t getMSP(void)
{
  __asm(" mrs r0, msp");
  __asm(" bx lr");
  return 0;
}

/* Name:setPSP
 * Parameters: 32-bit integer memory location
 * Returns:
 * Description: sets the PSP to the desired value
 */
void setPSP(volatile uint32_t newPSP)
{
  __asm(" msr psp, r0");
}

/* Name: setMSP
 * Parameters: 32-bit integer memory location
 * Returns:
 * Description: sets the MSP to the desired value
 */
void setMSP(volatile uint32_t newMSP)
{
  __asm(" msr msp,r0");
}

/* Name: getSP
 * Parameters:
 * Returns: 32-bit integer memory location
 * Description: returns the current Stack Pointer
 */
uint32_t getSP(void)
{
  __asm(" mov r0,SP");
  __asm(" bx lr");
  return 0;
}

/* Name: setMgmtArg
 * Parameters: 32-bit integer memory location
 * Returns:
 * Description: puts the requeted pointer into R7
 */
void setMgmtArg(volatile uint32_t arg)
{
  __asm(" mov r7,r0");
}
