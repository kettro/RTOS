/* File: supervisor.c
 * Author: Ross Kettleson
 * Revision Nov 14 2016
 * Purpose: Publicly accessible Supervisor calls
 */

#include "supervisor.h"
#include "syslib.h"
#include "process.h"
#include "alloc.h"
#include <stdlib.h>

pcb_t* current_process = NULL;
pcb_t* idle_process = NULL;
priority_e current_plevel = TOP_pl;
pcb_t* ProcessQHead[TOTAL_PLEVELS] = {NULL, NULL, NULL, NULL, NULL};

/* Name: mgmtGetID
 * Parameters:
 * Returns: returns the process' PID
 * Description: makes a supervisor call to get the current process' PID
 */
uint32_t mgmtGetID(void)
{
  volatile mgmt_arg_t id_arg = { .call_id = GETID_mc };
  setMgmtArg((uint32_t)&id_arg);
  SVC();
  return id_arg.retval;
}

/* Name: mgmtNice
 * Parameters: desired priority
 * Returns: error codes, if any
 * Description: changes the current process' priority
 *              to the requested level
 */
uint32_t mgmtNice(priority_e level)
{
  // Promote the process' priority to the next level
  // Or to the requested level: use an argument
  volatile mgmt_arg_t nice_arg = { .call_id = NICE_mc, .arg1 = level };
  setMgmtArg((uint32_t)&nice_arg);
  SVC();
  return nice_arg.retval;
}

/* Name: mgmtSwitch
 * Parameters:
 * Returns:
 * Description: context switches to the next process
 */
void mgmtSwitch(void)
{
  volatile mgmt_arg_t switch_arg = { .call_id = CONTEXTSWITCH_mc };
  setMgmtArg((uint32_t)&switch_arg);
  SVC();
}

/* Name: mgmtGetPriority
 * Parameters:
 * Returns: the current process' priority
 * Description: makes a supervisor call to retrieve and return
 *              the current process' priority
 */
priority_e mgmtGetPriority(void)
{
  volatile mgmt_arg_t pl_arg = { .call_id = GETPLEVEL_mc };
  setMgmtArg((uint32_t)&pl_arg);
  SVC();
  return (priority_e)(pl_arg.retval);
}

/* Name: mgmtTerm
 * Parameters:
 * Returns: error code if any
 * Description: terminates the current process
 */
uint32_t mgmtTerm(void)
{
  volatile mgmt_arg_t term_arg = { .call_id = TERM_mc };
  setMgmtArg((uint32_t)&term_arg);
  SVC();
  switch(term_arg.retval){
    case E_NO_CURRENT_PROCESS_mrv:
      //No Current Process Error during Terminate
      break;
    case GOOD_mrv:
      //Improper Good Retval Error during terminate
      break;
      // this is bad!!
      // this should never be reached
  }
  while(1);
}

/* Name: mgmtNew
 * Parameters: function to initialise in a thread
 *             disired priority level for the function
 * Returns:
 * Description: creates a new thread and executes the
 *              requested function. it places the function
 *              in the requested priority level
 */
void mgmtNew(void (*TargetFn)(), priority_e level)
{
  static uint8_t pid_counter = 0;
  pcb_t* pl_head;
  pcb_t* new_pcb = (pcb_t*)alloc(sizeof(pcb_t)); // allocate a PCB
  stackframe_t* new_stack = (stackframe_t*)alloc(STACKSIZE);
  if(new_stack == NULL){
    dealloc(new_pcb);
    return;
  }
  new_pcb->stack_base_ptr = (uint32_t)new_stack;
  new_stack = (stackframe_t*)((uint32_t)new_stack + STACKSIZE - sizeof(stackframe_t));
  new_stack->psr = 0x01000000; // thumb mode
  new_stack->pc = (uint32_t)TargetFn;
  new_stack->lr = (uint32_t)mgmtTerm;

//  new_stack->r0 = 0x00000000;
//  new_stack->r1 = 0x11111111;
//  new_stack->r2 = 0x22222222;
//  new_stack->r3 = 0x33333333;
//  new_stack->r4 = 0x44444444;
//  new_stack->r5 = 0x55555555;
//  new_stack->r6 = 0x66666666;
//  new_stack->r7 = 0x77777777;
//  new_stack->r8 = 0x88888888;
//  new_stack->r9 = 0x99999999;
//  new_stack->r10 = 0xAAAAAAAA;
//  new_stack->r11 = 0xBBBBBBBB;
//  new_stack->r12 = 0xCCCCCCCC;

  new_pcb->psp = (uint32_t)new_stack;
  new_pcb->pid = pid_counter++;

  pl_head = ProcessQHead[level];
  new_pcb->priority = level;

  // Plug the PCB into a PLevel
  if(pl_head == NULL){
    pl_head = new_pcb;
    new_pcb->next_pcb = new_pcb;
    new_pcb->prev_pcb = new_pcb;
  }else{
    new_pcb->next_pcb = pl_head;
    new_pcb->prev_pcb = pl_head->prev_pcb;
    pl_head->prev_pcb->next_pcb = new_pcb;
    pl_head->prev_pcb = new_pcb;
  }
  // if first process to be added
  if(current_process == NULL){
    current_process = new_pcb;
  }
  // Ensure we're operating at the highest plevel
  if(level < current_process->priority){
    current_process = new_pcb;
  }

  ProcessQHead[level] = pl_head;

}
