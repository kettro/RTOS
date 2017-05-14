#ifndef __SUPERVISOR_H__
#define __SUPERVISOR_H__

#include "data_structures.h"
#include <stdint.h>

#define PROCESSQMAX          16
#define TOTAL_PLEVELS        5

uint32_t mgmtGetID(void);
void mgmtSwitch(void);
uint32_t mgmtNice(priority_e);
uint32_t mgmtTerm(void);
void mgmtNew(void (*TargetFn)(), priority_e level);
priority_e mgmtGetPriority(void);

#endif
