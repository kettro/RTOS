/*
 * File: alloc.h
 * Author: Ross Kettleson
 * Revision: 09/26/16
 */
#ifndef ALLOC_H_
#define ALLOC_H_

#include <stdint.h>
#include <stdlib.h>

// Function Prototypes
void alloc_init(void);
void* alloc(uint16_t request);
void dealloc(void* pointer);

#endif
