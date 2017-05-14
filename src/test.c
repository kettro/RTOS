#include "test.h"
#include "supervisor.h"
#include "wm.h"
#include <string.h>
#include <stdio.h>

extern void delay(void);

// volatile priority_e pl_count = TOP_pl;
volatile char order_count = 0;

/* Name: testNiceLowToHigh
 * Parameters:
 * Returns:
 * Description: tests context switching and changing
 *              priority from a high priority to a lower
 *              priority in 3 processes
 */
void testNiceLowToHigh(void)
{
//  int pid;
//  static int switch_count = 0;
//  static priority_e pl_count = TOP_pl;
//  priority_e pl, next_pl;
//  window_name_e wid;
//  char str[32] = "hello";
//
//  // Set up
//  pid = mgmtGetID();
//  pl = mgmtGetPriority();
//  wid = wmRequestWindow();
//  next_pl = pl_count++;
//
//  // Print the present state
//  //sprintf(str, "ID: %d, Priority: %d, Switch#: %d, Next Priority: %d", pid, pl, switch_count, next_pl);
//  wmPuts(str, wid);
//  wmNewline(wid);
//
//  // let the other processes do so as well.
//  switch_count++;
//  //mgmtSwitch();
//  // Nice
//  mgmtNice(next_pl);
//
//  // Get and Print the current state
//  pid = mgmtGetID();
//  pl = mgmtGetPriority();
//  //sprintf(str, "ID: %d, Priority: %d, Switch#: %d, Nice#: %d", pid, pl, switch_count, order_count++);
//  wmPuts(str, wid);
//  wmNewline(wid);
//  switch_count++;
//  // Switch to another Process- though it should just return!
//  //mgmtSwitch();
//  // Print the current state
//  //sprintf(str, "ID: %d, Priority: %d, Switch#: %d, Nice#: %d", pid, pl, switch_count, order_count++);
//  wmPuts(str, wid);
//  wmNewline(wid);
//  //mgmtTerm();
//  // terminate, but hold onto the window
}

/* Name: testNiceHighToLow
 * Parameters:
 * Returns:
 * Description: test context switching and changing
 *              priority from a low priority to a higher
 *              Priority in 3 processes
 */
void testNiceHighToLow(void)
{
//  int pid;
//  priority_e pl, next_pl;
//  window_name_e wid;
//  char str[32];
//
//  // Set up
//  // get the PID
//  pid = mgmtGetID();
//  // get the Plevel
//  pl = mgmtGetPriority();
//  // acquire a window
//  wid = wmRequestWindow();
//  // Get next Plevel state
//  next_pl = (priority_e)(++pl_count);
//
//  // Print the current state:
//  sprintf(str, "ID: %d, Priority: %d, Switch#: %d, Next Priority: %d", pid, pl, switch_count, next_pl);
//  wmPuts(str, wid);
//  wmNewline(wid);
//  // Let other processes do the same
//  switch_count++;
//  mgmtSwitch();
//
//  // Print the current state:
//  sprintf(str, "ID: %d, Priority: %d, Switch#: %d, Nice#: %d", pid, pl, switch_count, order_count++);
//  wmPuts(str, wid);
//  wmNewline(wid);
//  // Nice
//  mgmtNice(next_pl);
//
//  // Get the current state
//  pid = mgmtGetID();
//  pl = mgmtGetPriority();
//
//  // Print it to the screen
//  // show the order count, to show the order that the thread runs
//  sprintf(str, "ID: %d, Priority: %d, Switch#: %d, Nice#: %d", pid, pl, switch_count, order_count++);
//  wmPuts(str, wid);
//  wmNewline(wid);
//  mgmtTerm();
//  // Here I would normally release the window,
//  // but for testing, hold on to it to keep the
//  // message on the screen.
//  // allow the thread to terminate
}

/* Name: testSimpleNice
 * Parameters:
 * Returns:
 * Description: Test only the functionality of NICE
 */
void testSimpleNice(void)
{
//  int pid = mgmtGetID();
//  char str[40];
//  int pl = mgmtGetPriority();
//  window_name_e wid = wmRequestWindow();
//  sprintf(str, "PID = %d, priority = %d", pid, pl);
//
//  wmPuts(str, wid);
//  wmNewline(wid);
//  delay();
//  delay();
//  delay();
//  mgmtNice((pid + 1) % 5);
//  delay();
//  delay();
//  delay();
//  pl = mgmtGetPriority();
//  sprintf(str, "PID = %d, Priority = %d, nice order = %d", pid, pl, order_count++);
//  wmPuts(str, wid);
//  wmNewline(wid);
//  delay(); // and terminate
//  mgmtTerm();
}

/* Name: testInputReceive
 * Parameters:
 * Returns:
 * Description: test input. Receive a string and repeat it back out
 */
void testInputReceive(void)
{
  char rxstr[80];
  window_name_e wid;
  wid = wmRequestWindow();
  wmGets(rxstr, 40, wid); // get an input
  wmNewline(wid); // \n\r
  wmPuts(rxstr, wid); // put the string out
  mgmtTerm();
}

/* Name: testSimpleContextSwitch
 * Parameters:
 * Returns:
 * Description: test the system's ability to context switch between processes
 */
void testSimpleContextSwitch(void)
{
  char msg[40];
  window_name_e wid = wmRequestWindow();
  int pid = mgmtGetID();
  static int counter = 0;
  while(counter < (10 + (2 * pid))){
    sprintf(msg, "PID: %d, WID: %d, COUNT: %d", pid, wid, counter++);
    wmPuts(msg, wid);
    wmNewline(wid);
    delay();
    delay();
    delay();
    delay();
  }
  wmReleaseWindow(wid);
}
