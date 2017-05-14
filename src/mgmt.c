/* File: mgmt.c
 * Author: Ross Kettleson
 * Revision Nov 14 2016
 */

#include "mgmt.h"
#include "process.h"
#include "alloc.h"
#include "systicklib.h"
#include "supervisor.h"
#include "syslib.h"
#include "msg.h"
#include "msg_data_structures.h"
#include <stdint.h>
#include <stdlib.h>

#include "test.h"

extern pcb_t* current_process;
extern pcb_t* ProcessQHead[TOTAL_PLEVELS];
extern volatile mcb_t* msg_directory[MCB_MAX];
extern void idle(void); // from threads.c

static void mgmtNICE_mc(mgmt_arg_t* mgmt_args);
static void mgmtCONTEXTSWITCH_mc(mgmt_arg_t* mgmt_args);
static void mgmtGETID_mc(mgmt_arg_t* mgmt_args);
static void mgmtGETPLEVEL_mc(mgmt_arg_t* mgmt_args);
static void mgmtTERM_mc(mgmt_arg_t* mgmt_args);
void mgmtSEND_mc(mgmt_arg_t* mgmt_args);
static void mgmtRECEIVE_mc(mgmt_arg_t* mgmt_args);
static void mgmtREGISTER_mc(mgmt_arg_t* mgmt_args);
static void mgmtCHECKMSGS_mc(mgmt_arg_t* mgmt_args);

void (* mgmtHandlers[9])(mgmt_arg_t* mgmt_args) = {
  mgmtCONTEXTSWITCH_mc,
  mgmtGETID_mc,
  mgmtGETPLEVEL_mc,
  mgmtNICE_mc,
  mgmtTERM_mc,
  mgmtSEND_mc,
  mgmtRECEIVE_mc,
  mgmtREGISTER_mc,
  mgmtCHECKMSGS_mc
};

/* Name: mgmt_ISR
 * Parameters:
 * Returns:
 * Description: ISR for the supervisor. Saves the registers
 *              of the calling process, then calls the supervisor
 *              handler (mgmt). On return from mgmt, restores the
 *              registers from either the PSP or MSP.
 */
void mgmt_ISR(void)
{
  __asm(" PUSH {LR}");          // Save the LR
  __asm(" TST LR,#4");          // Check for PSP/MSP ret

  __asm(" BNE ThreadReturn");   // If PSP, jump. Else:
  // MSP
  __asm(" PUSH {r4-r11}");      // Save the rest of the registers
  __asm(" MRS r0,MSP");         // 	move MSP to r0 as argument: arg_frame
  __asm(" BL mgmt");            // put the handler as the callback
  __asm(" POP {r4-r11}");       // get the registers from the stack
  __asm(" POP {PC}");           // pop and go
  // PSP
  __asm("ThreadReturn:");       // IF PSP::
  __asm(" MRS r0,PSP");         // put PSP into r0
  __asm(" STMDB r0!,{r4-r11}"); // store to the stack all of the regs
  __asm(" MSR PSP,r0");         // put r0 into the PSP
  __asm(" BL mgmt");            // set mgmt as the callback
  __asm(" MRS r0,PSP");         // move the PSP to r0
  __asm(" LDMIA r0!,{r4-r11}"); // get all the registers from Pstack
  __asm(" MSR PSP,r0");         // move value in r0 to PSP
  __asm(" POP {PC}");           // POP the PC and go
}

/* Name: mgmt
 * Parameters: a pointer to the stackframe of the calling process
 * Returns:
 * Description: Retrieves the value of the requested supervisor
 *              call, then calls that function.
 */
void mgmt(stackframe_t* arg_frame)
{
  // NOTE: no NULL guard for current_process. Pretty sure unneeded
  static truefalse_e mgmt_first_flag = TRUE;
  mgmt_arg_t* mgmt_args;

  if(mgmt_first_flag == TRUE){ // aka, first SV call
    mgmt_first_flag = FALSE;
    setPSP(current_process->psp + 8*sizeof(unsigned int));
    // setPSP(current_process->psp + 8*sizeof(uint32_t)); // double check this in debugger; int is of unreliable size
    SYSTICK_init(); // to ensure that it is initialized
    __asm(" movw LR, #0xFFFD");
    __asm(" movt LR, #0xFFFF");
    __asm(" bx LR");
  }else{ // aka, not the first SVcall
    mgmt_args = (mgmt_arg_t *)(arg_frame->r7);
    mgmt_args->retval = GOOD_mrv; // Good unless, well, not.
    mgmtHandlers[mgmt_args->call_id](mgmt_args);
  }
}

/* Name: mgmtNICE_mc
 * Parameters: arguments designating the desired priority level
 *             and the return value (for an error code)
 * Returns:
 * Description: Changes the priority of the current process to
 *              the desired priority. Changes the current process
 *              to the highest priority process. This is called only
 *              from mgmt
 */
static void mgmtNICE_mc(mgmt_arg_t* mgmt_args)
{
  priority_e plevel;
  priority_e curr_plevel = current_process->priority;
  pcb_t* QHead;
  uint8_t i; // counter
  plevel = (priority_e)mgmt_args->arg1;
  // curr_plevel = current_process->priority; // Done above, kept here as a reference
  // Check if new priority is same as current priority
  if(plevel == curr_plevel){ return; } // nothing to do
  if(plevel >= IDLE_pl){ return; } // disallowed
  // set the current priority level to the requested level

  QHead = ProcessQHead[plevel]; // Head of the requested plevel
  // Need to 1st change the position of the process
  // unlink from current queue, fix the queue

  /* Delinking Diagram:
   * ---------------------------------------------
   * Pp => current->prev_pcb
   * Pc => current
   * Pn => current->next_pcb
   * Double lines are the original node linkage,
   * Single lines the revised node linkage
   * ---------------------------------------------
   *              _____<______
   *  _____      /   _____    \     _____
   * |     | <<==== |     | <<==== |     |
   * |  Pp |        |  Pc |        |  Pn |
   * |_____| ====>> |_____| ====>> |_____|
   *            \______>_____/
   * ---------------------------------------------
   */
  // QHead is the head of the requested PLevel
  current_process->prev_pcb->next_pcb = current_process->next_pcb;
  current_process->next_pcb->prev_pcb = current_process->prev_pcb;
  ProcessQHead[curr_plevel] = current_process->next_pcb; // set the ProcessQHead for the old plevel
  if(ProcessQHead[curr_plevel] == current_process){ // was the only one left in the queue => NULLify
    ProcessQHead[curr_plevel] = NULL;
  }
  current_process->priority = plevel;
  // curr_plevel = plevel; // keep things synced up, unused hereafter
  // link into the new queue
  if(QHead == NULL){ // if an empty queue
    QHead = current_process; // ensure it clicks in properly to its queue
    ProcessQHead[plevel] = QHead; // sync the 2 back up after one is changed
    current_process->prev_pcb = current_process; // ensure that no NULL is happening
  }
  current_process->next_pcb = QHead; // put in behind the QHead
  current_process->prev_pcb = QHead->prev_pcb; // linking
  QHead->prev_pcb->next_pcb = current_process; // linking
  QHead->prev_pcb = current_process;

  // Need to determine if the current process should change
  // only change current process if there is something of more importance
  // count up from BOTTOM to the TOP
  for(i = 0;i < plevel; i++){
    if(ProcessQHead[i] != NULL){
      current_process->psp = getPSP();
      current_process = ProcessQHead[i];
      setPSP(current_process->psp);
      break;
    }
  }
    // check if anything in the current queue, or in the higher queues
    // if there are: then need to switch there
    // else, no context switch; return;
}

/* Name: mgmtCONTEXTSWITCH_mc
 * Parameters:
 * Returns:
 * Description: context switches to the next process
 */
static void mgmtCONTEXTSWITCH_mc(mgmt_arg_t* mgmt_args)
{
  current_process->psp = getPSP(); // save the PSP onto the current PCB
  current_process = current_process->next_pcb; // switch the current process
  ProcessQHead[current_process->priority] = current_process; // set the head of the current PL
  setPSP(current_process->psp); // recall the PSP from the PCB
}

/* Name: mgmtGETID_mc
 * Parameters: pointer to a structure containing the retval
 * Returns:
 * Description: returns to the retval the current process' ID
 */
static void mgmtGETID_mc(mgmt_arg_t* mgmt_args)
{
  mgmt_args->retval = current_process->pid;
}

/* Name: mgmtGETPLEVEL_mc
 * Parameters: pointer to a structure containing the retval
 * Returns:
 * Description: returns to the retval the current process' priority
 */
static void mgmtGETPLEVEL_mc(mgmt_arg_t* mgmt_args)
{
  mgmt_args->retval = current_process->priority;
}

/* Name: mgmtTERM_mc
 * Parameters: pointer to a structure containing the retval
 * Returns:
 * Description: Terminates a process. Frees its PCB and stack.
 *              Switches to the highest priority process
 */
static void mgmtTERM_mc(mgmt_arg_t* mgmt_args)
{
  // curr_plevel = current_process->priority; // Set above, kept here as a reference
  priority_e curr_plevel = current_process->priority;
  uint8_t i;
  pcb_t* PCB_to_term = current_process; // set the target
  if(current_process->priority == IDLE_pl){ return; } // can't kill the idle process

  /* TERM Diagram:
   * -----------------------------------------------------------------
   * where [0] => NULL pcb node, else => non-NULL pcb node:
   *   case A: where [X] is the current node: set and go to the PQH
   *   case B: where [Y] is the current node: set PQH, go to TOP PQH
   *   case C: where [Z] is the current node: NULL PQH, go to TOP PQH
   * -----------------------------------------------------------------
   * 0 >-[1]-[X]->
   * 1 >-[1]-[1]-[Y]->
   * 2 >-[Z]->
   * 3 >-[0]->
   * -----------------------------------------------------------------
   */

  if(current_process == current_process->next_pcb){ // Case C: Points to itself, is alone
    ProcessQHead[curr_plevel] = NULL; // ensure the PQ is updated
  }else{ // case A and B
    current_process->prev_pcb->next_pcb = current_process->next_pcb; // delink from the queue
    current_process->next_pcb->prev_pcb = current_process->prev_pcb;
    ProcessQHead[curr_plevel] = current_process->next_pcb;
  }
  // Free the current_process's crap
  dealloc((void*)(PCB_to_term->stack_base_ptr));
  dealloc(PCB_to_term);
  // Set the next process
  for(i = 0;i < TOTAL_PLEVELS; i++){
    if(ProcessQHead[i] != NULL){
      current_process = ProcessQHead[i];
      setPSP(current_process->psp); // context switch!
      return;
    }
  }
  mgmt_args->retval = E_NO_CURRENT_PROCESS_mrv;
}

void mgmtSEND_mc(mgmt_arg_t* mgmt_args)
{
  // always a non-blocking send
  msg_element_t* msg_to_send = (msg_element_t*)(mgmt_args->arg2);
  msg_element_t* msg; // shorthand
  pcb_t* qhead; // shorthand
  char i; // counter
  volatile mcb_t* dst_mcb = msg_directory[msg_to_send->dst_mid]; // shorthand
  char length = msg_to_send->msg_len;

  if(msg_to_send->src_mid != MCB_RESERVED && msg_directory[msg_to_send->src_mid]->pcb != current_process){ // illegal!
    // ISRs/Process without PCBs can send but cannot receive
    if(dst_mcb->pcb != NULL || dst_mcb->util_index != EMPTY_util){
      mgmt_args->retval = E_INVALID_SENDER_OR_RECEIVER_mrv;
      return;
    }
  }

  /* If the receiver is blocked:
   * The message can be directly passed, as the rx_process is waiting
   * determine the length to send (using the MAX size of the rx_args)
   * save the relevant values of the rx_args
   * copy over the message to the rx_args's character array
   * Need to relink the blocked process back into its queue
   */
  if(dst_mcb->blocked_rx == TRUE){ // receiver is blocked
    // just give the message to the guy
    // first, get the length
    length = (length < dst_mcb->rx_args.max_len) ? length : dst_mcb->rx_args.max_len;
    // then get the "From" mid
    dst_mcb->rx_args.src_mid = msg_to_send->src_mid;
    // copy over the length
    dst_mcb->rx_args.max_len = length;
    dst_mcb->blocked_rx = FALSE; // unset being blocked
    // copy over
    memcpy(*(dst_mcb->rx_args.msg), msg_to_send->message, length);
    dst_mcb->rx_args.dirty_bit = 1;
    // Relink the blocking process into the priority queue
    qhead = ProcessQHead[dst_mcb->pcb->priority];
    if(qhead == NULL){
      qhead = dst_mcb->pcb;
      dst_mcb->pcb->prev_pcb = dst_mcb->pcb;
      ProcessQHead[dst_mcb->pcb->priority] = qhead;
    }
    dst_mcb->pcb->next_pcb = qhead;
    dst_mcb->pcb->prev_pcb = qhead->prev_pcb;
    qhead->prev_pcb->next_pcb = dst_mcb->pcb; // prev to the head
    qhead->prev_pcb = dst_mcb->pcb;
  }
  /* If the receiver is not blocked:
   * The Rx_process is busy, and the message must be enqueued
   * allocate a message and copy the message info into it
   */
  else{ // receiver is not blocked
    // allocate and copy into a new msg item
    msg = (msg_element_t*)alloc(sizeof(msg_element_t));
    memcpy(msg, msg_to_send, sizeof(msg_element_t));

    if(dst_mcb->RxQ[dst_mcb->RxQHead] == NULL){
      dst_mcb->RxQ[dst_mcb->RxQHead] = msg;
    }else{
      dst_mcb->RxQ[dst_mcb->RxQTail] = msg;
    }
    dst_mcb->RxQTail = (dst_mcb->RxQTail + 1) % RXQ_MAX; // wrap around
  }
  // recalculate the current process
  for(i = 0; i < TOTAL_PLEVELS; i++){
    if(ProcessQHead[i] != NULL){
      current_process->psp = getPSP();
      current_process = ProcessQHead[i];
      setPSP(current_process->psp);
      break;
    }
  }
}

static void mgmtRECEIVE_mc(mgmt_arg_t* mgmt_args)
{
  // always a blocking receive: if no message, must block to wait for one
  // receive_msg holds the arguments supplied by, and returned to, the process
  recv_t* rx_args = (recv_t*)(mgmt_args->arg2);
  uint8_t len_to_cpy = rx_args->max_len;
  msg_element_t* target_msg;
  pcb_t* dst;
  char i;
  volatile mcb_t* dst_mcb = msg_directory[rx_args->dst_mid];
  if(dst_mcb->pcb != current_process){
    mgmt_args->retval = E_INVALID_SENDER_OR_RECEIVER_mrv;
    return;
  }

  /* if the queue is empty:
   * need to block.
   * Save the receive arguments in the MCB
   * Pull out of the Priority Queue
   * Set as blocking
   */
  if(dst_mcb->RxQ[dst_mcb->RxQHead] == NULL){ // nothing on the queues; block
    dst_mcb->blocked_rx = TRUE; // set as being blocked
    dst = dst_mcb->pcb; // shorthand
    dst_mcb->rx_args = *rx_args; // set the rx args
    // delink from the current priority queue;
    dst->prev_pcb->next_pcb = dst->next_pcb;
    dst->next_pcb->prev_pcb = dst->prev_pcb;
    // fix the Priority Q
    ProcessQHead[dst->priority] = dst->next_pcb;
    if(ProcessQHead[dst->priority] == dst){
      ProcessQHead[dst->priority] = NULL;
    }
    // now are blocking
    // when re-woken, looks for the value on rx_msg in the mcb
  }
  /* If the queue has items:
   * Get the first item in the queue
   * determine the length of the message to receive
   * copy the message over into the destination char array
   * dealloc the message
   * increment the Head to the next item, ensure staying in the queue
   */
  else{ // queues have some messages to get
//    if(dst_mcb->rx_args.dirty_bit){
//      dst_mcb->blocked_rx = FALSE;
//      return;
//    }

    dst_mcb->blocked_rx = FALSE;
    dst_mcb->rx_args.dirty_bit = 0;
    target_msg = dst_mcb->RxQ[dst_mcb->RxQHead]; // shorthand
    dst_mcb->rx_args = *rx_args; // OR!! just copy directly to the rx_args
    len_to_cpy = (len_to_cpy < target_msg->msg_len) ? len_to_cpy : target_msg->msg_len;
    memcpy(*(dst_mcb->rx_args.msg), target_msg->message, len_to_cpy); // copy over
    dealloc(target_msg); // remove the message
    dst_mcb->RxQ[dst_mcb->RxQHead] = NULL;
    dst_mcb->RxQHead = (dst_mcb->RxQHead + 1) % RXQ_MAX; // increment the head to the next position
    dst_mcb->rx_args.max_len = len_to_cpy;
    return;
  }

  for(i = 0; i < TOTAL_PLEVELS; i++){
    if(ProcessQHead[i] != NULL){
      current_process->psp = getPSP();
      current_process = ProcessQHead[i];
      setPSP(current_process->psp);
      break;
    }
  }
}

static void mgmtREGISTER_mc(mgmt_arg_t* mgmt_args)
{
  char mid = 0, i;
  volatile mcb_t* x = msg_directory[mid];
  while(x != NULL){
    mid = (mid+1) % MCB_MAX;
    x = msg_directory[mid];
  }
  // set defaults
  x = (mcb_t*)alloc(sizeof(mcb_t));
  msg_directory[mid] = x;
  msg_directory[mid]->pcb = current_process;
  msg_directory[mid]->RxQHead = 0;
  msg_directory[mid]->RxQTail = 0;
  msg_directory[mid]->util_index = EMPTY_util;
  msg_directory[mid]->blocked_rx = FALSE;
  msg_directory[mid]->rx_args.dirty_bit = 0;
  mgmt_args->retval = mid;
  for(i = 0; i < RXQ_MAX; i++){
    msg_directory[mid]->RxQ[i] = NULL;
  }

}

static void mgmtCHECKMSGS_mc(mgmt_arg_t* mgmt_args)
{
  char mid = mgmt_args->arg1; // get the mid in question
  volatile mcb_t* mcb = msg_directory[mid];
  mgmt_args->retval = (mcb->RxQ[mcb->RxQHead] != NULL); // if NULL: FALSE
}

/* Name: mgmt_init
 * Parameters:
 * Returns:
 * Description: Sets the PendSV interrupt as the lowest
 *              priority interrupt and adds the idle process
 *              to the Process Queue
 */
void mgmt_init(void)
{
  NVIC_SYS_PRI3_R |= PENDSV_BOT_PRI; // set to bottom priority
  // Spin off the Basic Processes
//  mgmtNew(idle, IDLE_pl);
}

/* Name: PendSV_SIR
 * Parameters:
 * Returns:
 * Description: context switches to the next process
 */
void PendSV_ISR(void)
{
  // needs to launch the context switch
  saveRegs();
  current_process->psp = getPSP();
  current_process = current_process->next_pcb;
  ProcessQHead[current_process->priority] = current_process; // set the head
  setPSP(current_process->psp);
  restoreRegs();
}
