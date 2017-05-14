/* File: UARTLib.c
 * Author: Ross Kettleson
 * Revision Oct 16 2016
 */

#include "uartlib.h"
#include "syslib.h"
#include <string.h>
#include "uart0_config.h" // register constants

volatile char is_txing = FALSE;
volatile strpack_t TxQ[NUM_TXQUEUES];
volatile strpack_cnt_t TxQ_index = { .head = 0, .tail = NUM_TXQUEUES-1, .str = 0 };

extern void RxEnqueue(char inchar);

/* Function: UART0_ISR
 * Parameters: 
 * Returns: 
 * Description: Interrupt Routine for the UART0. Enqueues received characters.
 *              If it has been transmitting, retrieves the next vahracter to send
 *              it it exists.
 */
void UART0_ISR(void)
{
  // if TX:
/*                    TxQ   .   str              * >> *                   TxQ    .  str
 *                    ____                       * >> *                   ____
 *  TxQ_index.head==>|__X_| ==>"str"             * >> *                  |____|
 *                   |__X_|       ^TxQ_index.str * >> * TxQ_index.head==>|__X_| ==>"ing"
 *  TxQ_index.tail==>|__X_|                      * >> * TxQ_index.tail==>|__X_|     ^TxQ_index.str
 *                   |____|                      * >> *                  |____|
 */
  if (UART0_MIS_R & UART_INT_TX){
    UART0_ICR_R |= UART_INT_TX; // Clear interrupts, allow for another to go out
    is_txing = TRUE;
    if(TxQ[TxQ_index.head].str[TxQ_index.str] != '\0'){
      UART0_DR_R = TxQ[TxQ_index.head].str[TxQ_index.str]; // char from the topmost string in the Q
      TxQ[TxQ_index.head].str[TxQ_index.str++] = '\0'; // Nullify that char
    }else{ // end of the string, ==  '\0'
      TxQ_index.len--; // decrement the number of strings in the queue
      TxQ_index.str = 0; // set looking at beginning of the string
      if(TxQ_index.len == 0){ // if we are all done
        is_txing = FALSE; // The queue is empty, clear the flag
        TxQ_index.head++;
        TxQ_index.head &= NUM_TXQUEUES - 1;
        return;
      }else{ // Queue isn't empty- still items to go
        TxQ_index.head++; // go to the next string in queue
        TxQ_index.head &= NUM_TXQUEUES-1; // ensure wrap around
        UART0_DR_R = TxQ[TxQ_index.head].str[TxQ_index.str]; // send it out
        TxQ[TxQ_index.head].str[TxQ_index.str++] = '\0'; // Nullify
      }
    }
  }
  // else: Has been receiving
  if (UART0_MIS_R & UART_INT_RX){
    UART0_ICR_R |= UART_INT_RX; // clear interrupts
    RxEnqueue(UART0_DR_R);
  }
}

/* Function: UART0_init
 * Parameters: 
 * Returns: 
 * Description: initializes the UART0
 */
void UART0_init(void)
{
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0; // Enable Clock Gating for UART0
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // Enable Clock Gating for PORTA
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // Disable the UART
  // Setup the BAUD rate
  UART0_IBRD_R = 8;	// IBRD = int(16,000,000 / (16 * 115,200)) = int(8.68055)
  UART0_FBRD_R = 44;	// FBRD = int(0.68055 * 64 + 0.5) = 44.0552
  UART0_LCRH_R = (UART_LCRH_WLEN_8); // WLEN: 8, no parity, one stop bit, without FIFOs)
  UART0_CTL_R = UART_CTL_UARTEN;        // Enable the UART and End of Transmission Interrupts
  GPIO_PORTA_AFSEL_R |= EN_RX_PA0 | EN_TX_PA1;    // Enable Receive and Transmit on PA1-0
  GPIO_PORTA_DEN_R |= EN_DIG_PA0 | EN_DIG_PA1;   // Enable Digital I/O on PA1-0
  NVIC_EN0_R |= 1 << UART0_INTVECT; // enable vector in the NVIC
  UART0_IM_R |= (UART_INT_RX | UART_INT_TX); // Enable the RX/TX interrupt
}

/* Function: UART0_write
 * Parameters: data: character to be written to the UART
 * Returns: 
 * Description: Adds a string into the trasmisson queue.
 *              kickstarts the UART transmission if it is idle.
 */
void UART0_write(char* string)
{
  // check the length of the string, truncate&fix if too long
  uint8_t len = strlen(string);
  char temp;
  if(len > BASE_STR_LENGTH){
    len = BASE_STR_LENGTH;
    string[BASE_STR_LENGTH-1] = '\0'; // add nul character to ensure nul-termination
  }
  while(TxQ_index.len == NUM_TXQUEUES); // Mutex, as only otherwise changed in the ISR

  intDisable(); // ensure mutex when writing to the queues
  TxQ_index.tail++; // move the tail to the next available position, should loop around
  TxQ_index.tail &= NUM_TXQUEUES - 1;
  strncpy((char*)TxQ[TxQ_index.tail].str, string, len); // cast is so that it compiles, I hope it saves...
  TxQ_index.len++;

  if(is_txing == FALSE){ // kickstart
    temp = TxQ[TxQ_index.head].str[TxQ_index.str];
    TxQ[TxQ_index.head].str[TxQ_index.str++] = '\0';
    intEnable();
    UART0_DR_R = temp;
  }else{ intEnable(); }
}
