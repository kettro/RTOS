#include "msg.h"
#include "msg_data_structures.h"
#include "alloc.h"
#include "process.h"
#include "syslib.h"
#include <string.h>
#include <stdio.h>

extern pcb_t* current_process;
static mid_t util_directory[NUM_OF_UTILS];
volatile mcb_t* msg_directory[MCB_MAX];

void msg_init(void)
{
  char i;
  for(i = 0; i < MCB_MAX; i++){
    msg_directory[i] = NULL;
  }
  for(i = 0; i < NUM_OF_UTILS; i++){
    util_directory[i] = EMPTY_MID;
  }
}

void msgSend(mid_t dst, mid_t src, char* msg, uint8_t len)
{
  volatile msg_element_t message;
  message.dst_mid = dst;
  message.src_mid = src;
  message.msg_len = len;
  char i;
  for(i = 0; i < len; i++){
    message.message[i] = msg[i];
  }
//  memcpy((void*)message.message, msg, len);
  volatile mgmt_arg_t send_arg = {
      .call_id = SEND_mc,
      .arg2 = (uint32_t)&message,
  };
  setMgmtArg((uint32_t)&send_arg);
  SVC();
}

uint8_t msgReceive(mid_t dst, mid_t* src, char* msg, uint8_t max_len)
{
  volatile recv_t rx_args = {
    .dst_mid = dst,
    .msg = &msg,
    .max_len = max_len
  };

  volatile mgmt_arg_t receive_arg = {
    .call_id = RECEIVE_mc,
    .arg2 = (uint32_t)&rx_args,
    .retval = 0
  };
  setMgmtArg((uint32_t)&receive_arg);
  SVC();

  // After the Kernel:
  *src = msg_directory[dst]->rx_args.src_mid;
  return msg_directory[dst]->rx_args.max_len;
}

mid_t msgRegister(void)
{
  volatile mgmt_arg_t reg_arg = {
      .call_id = REGISTER_mc
  };
  setMgmtArg((uint32_t)&reg_arg);
  SVC();
  return reg_arg.retval;
}

truefalse_e msgCheckForMessages(mid_t mid)
{
  volatile mgmt_arg_t check_arg = {
      .call_id = CHECKMSGS_mc,
      .arg1 = mid
  };
  setMgmtArg((uint32_t)&check_arg);
  SVC();
  return (truefalse_e)check_arg.retval;
}

void msgIndex(utils_e utility, mid_t self)
{
  if(util_directory[utility] != EMPTY_MID){
    // error: already indexed
  }else{
    util_directory[utility] = self; // register in the directory
    msg_directory[self]->util_index = utility; // register in the mcb
  }
}

mid_t msgQueryByUtil(utils_e utility)
{
  return util_directory[utility];
}

utils_e msgQueryByMid(mid_t mid)
{
  return msg_directory[mid]->util_index;
}
