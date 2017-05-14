/* File: systicklib.h
 * Author: Ross Kettleson
 * Revision Oct 16 2016
 */
#ifndef __SYSTICKLIB_H__
#define __SYSTICKLIB_H__

#include <stdint.h>
#include "data_structures.h"

void SYSTICK_ISR(void);
void SYSTICK_init(void);

#endif

