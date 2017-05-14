#ifndef __MSG_DATA_STRUCTURES_H__
#define __MSG_DATA_STRUCTURES_H__
#include "data_structures.h"
#include <stdint.h>

#define RXQ_MAX       8
#define MSG_MAX_LEN   32
#define MCB_MAX       24
#define MCB_RESERVED  MCB_MAX + 1 // for ISRs
#define EMPTY_MID     MCB_MAX + 2 // as a default

typedef uint8_t mid_t;

#define NUM_OF_UTILS    10
typedef enum KNOWN_UTILITIES_E_{
  SPI_util,
  UART_util,
  GPIOF_util,
  TRAIN_EXPRESS_util,
  TRAIN_LOCAL_util,
  TRAIN_TX_util,
  TRAIN_RX_util,
  HALL_util,
  CONDUCTOR_util,
  IDLE_util,
  EMPTY_util
}utils_e;

typedef struct conductor_cmd_t_{
  char id;
  char cmd;
  char arg1;
  char arg2;
  char arg3;
}conductor_cmd_t;

// msgElement
typedef struct msg_element_t_{
  pcb_t*    pcb; // Sending PCB
  char      message[MSG_MAX_LEN];
  uint8_t   msg_len;
  uint8_t   dst_mid;
  uint8_t   src_mid;
}msg_element_t;

typedef struct receive_args_t_{
  char    dst_mid;
  char    src_mid;
  char**  msg;
  char    max_len;
  char    dirty_bit;
}recv_t;
// MCB
typedef struct mcb_t_{
  pcb_t*          pcb; // receiving PCB
  truefalse_e     blocked_rx;
  msg_element_t*  RxQ[RXQ_MAX];
  uint8_t         RxQHead;
  uint8_t         RxQTail;
  utils_e         util_index; // not wasting any space here, oddly
  recv_t          rx_args;
}mcb_t;

#endif
