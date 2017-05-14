#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#include <stdint.h>

#define reg_t(reg)    (*((volatile uint32_t *)reg))

typedef struct _pcb_t{
  uint32_t psp;              // 4 bytes
  uint32_t stack_base_ptr;   // 4 bytes
  struct _pcb_t* next_pcb;   // 4 bytes
  struct _pcb_t* prev_pcb;   // 4 bytes
  uint8_t pid;               // 1 byte
  priority_e priority;       // 1 byte
  uint8_t mid;               // 1 byte
}pcb_t;

typedef struct _mgmt_args_t{
  uint8_t call_id;
  uint8_t retval;
  uint8_t arg1;
  uint32_t arg2;
}mgmt_arg_t;

typedef struct _stackframe_t{ // 64 Bytes
  // SW saved  // 8 * 4 = 32 bytes
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  // HW saved
  uint32_t r0;  // 8 * 4 = 32 bytes
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t pc;
  uint32_t psr;
}stackframe_t;

#endif
