/* File: UARTLib.h
 * Author: Ross Kettleson
 * Revision Oct 26 2016
 */
#ifndef __UARTLIB_H__
#define __UARTLIB_H__

#include <stdint.h>
#include "data_structures.h"
#include "window_data_structures.h"

void UART0_ISR(void);
void UART0_init(void);
void UART0_write(char*);


#endif
