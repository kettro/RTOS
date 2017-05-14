/* File: wm.c
 * Author: Ross Kettleson
 * Revision Nov 14 2016
 * Purpose: Window Management and Public I/O Access
 */
#include "wm.h"
#include "uartlib.h"
#include "syslib.h"
#include <string.h> // for strncpy
#include <stdio.h>
#include "window_data_structures.h"
#include "supervisor.h"
#include <strings.h>

extern volatile char is_txing;
extern volatile strpack_t TxQ[NUM_WINDOWS];
extern volatile strpack_cnt_t TxQ_index;

static char RxDequeue(window_name_e wid);
extern void RxEnqueue(char inchar);

typedef struct _window_t_{ // 40 bytes
  truefalse_e active;      // whether the window is allocated to a process
  cursor_t wcursor;        // the window's cursor
  cursor_t home;           // the origin point of the window (masked as 0,0)
  cursor_t dimensions;     // the dimensions of the screen
  strpack_t RxQ;           // receive UART queue
  uint8_t RxQ_length;
  uint8_t RxQ_head;
  uint8_t RxQ_tail;        // the tail of the UART RX queue
}window_t;

// Globals

cursor_t mcursor; // master cursor
window_t window[NUM_WINDOWS]; // the Windows themselves
window_name_e current_input_source;


/* Name: wm_init
 * Parameters:
 * Returns:
 * Description: Set defaults, and construct the Screen.
 */
void wm_init(void)
{
  // set defaults for the Windows
  int i;
  for (i=0;i<NUM_WINDOWS;i++){
    window[i].active = FALSE;
    window[i].dimensions.x = DEFAULT_WINDOWX;
    window[i].dimensions.y = DEFAULT_WINDOWY;
    window[i].home.x = 1;
    window[i].home.y = i * 10 + 1 + i; // every other 10, with 1 offset of top, and i offset for a border
    window[i].wcursor = window[i].home;
  }
  mcursor.x = 1;
  mcursor.y = 1;
  current_input_source = NONE_wn;

  // Draw the window borders
  char borders[DEFAULT_WINDOWX];
  for(i=0;i<DEFAULT_WINDOWX;i++){
    borders[i] = '-';
  }
  borders[i-1] = '\0';
  // Write to the Screen
  for(i=0;i<NUM_WINDOWS;i++){
    wmMoveCursor(window[i].home.x, window[i].home.y + window[i].dimensions.y);
    UART0_write(borders);
  }
  // return to the 1,1 of the master cursor
  wmMoveCursor(1,1);
  // set scrolling region to the top window
  wmSetScroll(TOP_wn);

  // debug:
  // window[0].active = TRUE;
}

/* Name: wmRequestWindow
 * Parameters:
 * Returns: window ID corresponding to the allocated window
 * Description: Block until a window becomes available
 *              Context switch away if none are at the moment.
 */
window_name_e wmRequestWindow(void)
{
  window_name_e wid;
  while(1){
    for(wid = TOP_wn; wid < NUM_WINDOWS; wid++){
      if(window[wid].active == FALSE){
        window[wid].active = TRUE;
        return wid;
      }
    }
    mgmtSwitch();
  }
}

/* Name: wmReleaseWindow
 * Paramters: Window ID to be released
 * Returns:
 * Description: Resets the specified window to being inactive.
 *              Clears the screen of the last process
 */
void wmReleaseWindow(window_name_e wid)
{
  window[wid].active = FALSE;
  window[wid].wcursor = window[wid].home;
  int i;
  // clear the screen
  wmMoveCursor(window[wid].home.x, window[wid].home.y); // go home
  systickDisable(); // mutex
  for(i = 0; i < window[wid].dimensions.y; i++){
    UART0_write("\x1b[2K\n"); // clear the screen, then move to next line
  }
  wmMoveCursor(window[wid].home.x, window[wid].home.y);
  systickEnable(); // release mutex
}

/* Name: wmPuts
 * Paramters: String to be printed
 *            Window ID of the window to print to
 * Returns:
 * Description: Print a string to the specified window
 */
void wmPuts(char* str, window_name_e wid)
{
  // need to mutex this
  // problem here is that if the queue only has <=1 spot, with ints disabled, it won't work.
  // this is ugly, and bad, but I need to make sure it stays atomic.
  // Not really a good way to do this otherwise.
  systickDisable();
  window_t* w = &window[wid];
  // if the current is not the same as the window's
  if(mcursor.x != w->wcursor.x || mcursor.y != w->wcursor.y){
    wmSetScroll(wid); // set the scroll region first- resets the cursor
    wmMoveCursor(w->wcursor.x, w->wcursor.y); // Then move the cursor
  }
  UART0_write(str);
  // update the window cursor
  w->wcursor.x = w->wcursor.x + strlen(str);
  if(w->wcursor.x > w->dimensions.x){
    if(w->wcursor.y <= (w->home.y + w->dimensions.y - 1)){
      w->wcursor.y++;
    }
    w->wcursor.x -= w->dimensions.x; // wrapped around on the next line
  }
  // update the master cursor
  mcursor = w->wcursor;
  systickEnable();
}

/* Name: wmGets
 * Parameters: destination string to put the received string
 *             length of the desired string
 *             window ID of the window to receive from
 * Returns:
 * Description: Attempts to acquire the current input source, blocks until it does
 *              Returns the data received from the UART, blocks if the Queue is empty
 *              Releases the current input source when done.
 */
void wmGets(char* dst, uint8_t length, window_name_e wid)
{
// the RX queue provides a buffer to retrieve from
// if asking for more than is there, context switch until it is there.
// if current is the active input source, then good
// else move the cursor to the correct area (wcursor)
  wmNewline(wid); // moves the cursor, and puts it on a newline
  wmPuts(">> ", wid); // adds a cursor to indicate input

  // ready to scan for some inputs
  int i; // counter
  char rxch; // received character
  char rxstr[BASE_STR_LENGTH]; // receive string
  // acquire the current input
  // if it is unheld, then can move on, else, block
  while(1){
    intDisable();
    if(current_input_source == NONE_wn){
      current_input_source = wid;
      intEnable();
      break;
    }
    intEnable();
    mgmtSwitch(); // context switch away if not current input source
  }
  // receive characters
  // if the character is a CR or if it overflows the buffer, then you've gone too far.
  // if CR: the item must be interpreted
  // if BS: delete the last item in the string, if there is one
  // i is incremented to the next char spot after receiving the character
  i = 0;
  do{
    rxch = RxDequeue(wid);
    if(rxch == '\r'){
      if(i >= length){ rxstr[length-1] = '\0'; } // truncate the string
      break;
    } // not using switch as inside of do/while statement
    else if(rxch == '\b' || rxch == 127){ // backspace or delete
      if(i == 0){ continue; }
      rxstr[--i] = '\0';
    }else{
      rxstr[i++] = rxch; // otherwise, just store the character
    }
  }while(rxch != '\r' || i < length);

  strncpy(dst, rxstr, length); // copy to the destination; make available
  intDisable();
  current_input_source = NONE_wn; // release the current input on completion
  intEnable();
}

/* Name: wmNewline
 * Parameters: window ID of the window to print to
 * Returns:
 * Description: method for safely printing a newline to the screen
 */
void wmNewline(window_name_e wid)
{
  // context switch guards
  window_t* w = &window[wid];
  if(mcursor.x != w->wcursor.x || mcursor.y != w->wcursor.y){
    wmSetScroll(wid); // set the scroll region first?
    wmMoveCursor(w->wcursor.x, w->wcursor.y); // Then move the cursor?
  }

  UART0_write("\n");
  UART0_write("\r");
  if(w->wcursor.y < (w->home.y + w->dimensions.y - 1)){
    w->wcursor.y++;
  }
  w->wcursor.x = 0;
  mcursor = w->wcursor;
}

/* Name: wmMoveCursor
 * Parameters: x and y coordinates to move the cursor to
 * Returns:
 * Description: move the cursor to the specified location
 *              updates the master cursor to this location
 */
void wmMoveCursor(uint8_t x, uint8_t y)
{
  char move_command[8];
  sprintf(move_command, "\x1b[%d;%dH", y, x); // because it is apparently backwards
  // Set the master cursors
  mcursor.x = x;
  mcursor.y = y;
  UART0_write(move_command);
}

/* Name: RxDequeue
 * Parameters: window ID of the window to retrieve from
 * Returns: a character received from the Receive Queue
 * Description: Attempts to get a character from the Window's
 *              receive queue. Blocks until the queue has data in it.
 */
static char RxDequeue(window_name_e wid)
{
  // Mutex
  // get the head of the RxQ
  char echo[2] = {'\0', '\0' };
  while(1){
    while(window[wid].RxQ_length == 0);
    intDisable();
    if((echo[0] = window[wid].RxQ.str[window[wid].RxQ_head]) != '\0'){ break; }
    intEnable();
  }
  // interrupts still disabled
  window[wid].RxQ.str[window[wid].RxQ_head++] = '\0';
  window[wid].RxQ_head &= BASE_STR_LENGTH;
  window[wid].RxQ_length--;
  intEnable();
  wmPuts(echo, wid); // echo it out
  return echo[0];
}

/* Name: RxEnqueue
 * Parameters: character to be enqueued
 * Returns:
 * Description: Enqueues a character onto the receive queue of the
 *              current input source.
 */
void RxEnqueue(char inchar)
{
  // already mutexed
  window_name_e wid = current_input_source;
  window_t* w = &window[wid];
  // add to the current window's queue;
  w->RxQ.str[w->RxQ_tail++] = inchar;
  w->RxQ_tail &= BASE_STR_LENGTH;
  // if too long, no increase in length, just squashing
  if(w->RxQ_length < BASE_STR_LENGTH) { w->RxQ_length++; }
}

/* Name: wmSetScroll
 * Parameters: window ID of the window to set the scroll of
 * Returns:
 * Description: sets the designated window as a scrolling region
 *              will reset the cursor to the 1,1 location, so must be
 *              used carefully
 */
void wmSetScroll(window_name_e wid)
{
  char scroll_command[8];
  window_t w = window[wid];
  sprintf(scroll_command, "\x1b[%d;%dr", w.home.y, w.home.y + w.dimensions.y - 1);
  UART0_write(scroll_command);
}
