#ifndef __MSG_H__
#define __MSG_H__
#include <stdint.h>
#include "data_structures.h"
#include "msg_data_structures.h"

void msg_init(void);
void msgSend(mid_t dst, mid_t src, char* msg, uint8_t len);
uint8_t msgReceive(mid_t dst, mid_t* src, char* msg, uint8_t max_len);
mid_t msgRegister(void);
void msgIndex(utils_e utility, mid_t self);
mid_t msgQueryByUtil(utils_e utility);
utils_e msgQueryByMid(mid_t mid);
truefalse_e msgCheckForMessages(mid_t mid);

#endif
