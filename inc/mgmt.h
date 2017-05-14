#ifndef __MGMT_H__
#define __MGMT_H__

#include "data_structures.h"

// MGMT
void mgmt_ISR(void);
void mgmt(stackframe_t* arg_frame);
void mgmt_init(void);

#endif
