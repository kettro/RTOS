#include "threads.h"
#include "supervisor.h"
#include "wm.h"
#include "test.h"
#include <string.h>
#include "msg.h"
#include <stdio.h>
#include "msg_data_structures.h"
#include "train_control.h"
#include "train_types.h"

void msg_testing(void);
void spi_testing(void);
void switch_testing(void);
extern void controllerServer(void);
extern void hallServer(void);
extern void trainServer(void);

/*
 * Name: createThreads
 * Parameters:
 * Return:
 * Description: creates a number of threads for the current application.
 */
void createThreads(void)
{
#ifdef TEST_SEQUENCE
#ifdef TEST_NICE_HIGH_TO_LOW
  mgmtNew(testNiceHighToLow, TOP_pl);
  mgmtNew(testNiceHighToLow, TOP_pl);
  mgmtNew(testNiceHighToLow, TOP_pl);
#endif // TEST_NICE_HIGH_TO_LOW
#ifdef TEST_NICE_LOW_TO_HIGH
  mgmtNew(testNiceLowToHigh, BOTTOM_pl);
//  mgmtNew(testNiceLowToHigh, BOTTOM_pl);
//  mgmtNew(testNiceLowToHigh, BOTTOM_pl);
#endif // TEST_NICE_HIGH_TO_LOW
#ifdef TEST_INPUT_RECEIVE
  mgmtNew(testInputReceive, TOP_pl);
#endif // TEST_INPUT_RECEIVE
#ifdef TEST_SIMPLE_CONTEXT_SWITCH
  mgmtNew(testSimpleContextSwitch, TOP_pl);
  mgmtNew(testSimpleContextSwitch, TOP_pl);
  mgmtNew(testSimpleContextSwitch, TOP_pl);
  mgmtNew(testSimpleContextSwitch, TOP_pl);
#endif // TEST_SIMPLE_CONTEXT_SWITCH
#ifdef TEST_SIMPLE_NICE
  mgmtNew(testSimpleNice, ONE_pl);
  mgmtNew(testSimpleNice, ONE_pl);
  mgmtNew(testSimpleNice, ONE_pl);
#endif
#endif // TEST_SEQUENCE

//  mgmtNew(msg_testing, TOP_pl);
//  mgmtNew(msg_testing, TOP_pl);
//  mgmtNew(spi_testing, THREE_pl);
//  mgmtNew(switch_testing, THREE_pl);
  mgmtNew(controllerServer, TOP_pl);
  mgmtNew(hallServer, ONE_pl);
  mgmtNew(trainServer, TWO_pl); // express
  //mgmtNew(trainServer, TWO_pl); // local
  mgmtNew(idle, IDLE_pl);

}

/*
 * Name: idle
 * Parameters:
 * Return:
 * Description: Idle Process, run only when the queue is fully empty.
 */
void idle(void)
{
  while(1);
//  window_name_e wid = wmRequestWindow();
//  wmPuts("Idling", wid);
//  wmNewline(wid);
//  while(1){
//    delay();
//    delay();
//    wmPuts("I\r", wid); // show a "cursor"
//    delay();
//    delay();
//    wmPuts(" \r", wid); // delete it
//  }
}

void msg_testing(void)
{
  int pid = mgmtGetID();
  mid_t mid = msgRegister();
  mgmtSwitch();

  msgSend((pid)%2, mid, "hello", 6);
  mid_t received_from_mid;
  char received_msg[MSG_MAX_LEN];
  char len = msgReceive(mid, &received_from_mid, received_msg, MSG_MAX_LEN);
  printf("pid = %d, mid = %d, from = %d, msg = %s\r\n", pid, mid, received_from_mid, received_msg);
  while(1);
}

void spi_testing(void)
{
  mid_t mid = msgRegister();
  msgIndex(CONDUCTOR_util, mid);
  char magdir;
  truefalse_e ec;
  char dir = 0;
  char mag = 0;
//  char msg[10];
//  mid_t src;
//  char len = 10;

////  trainTrainCtrlAll(mid, CW_tmd, MAX_tmd);
////  trainTrainCtrlAllRx(mid, &ec, &magdir);
//  while(1){
//    len = msgReceive(mid, &src, (char*)&msg, len);
////    printf("response received from %d: ec = %d, len = %X\n", src, ec, len);
//    if(msg[0] == 5){
//      trainTrainCtrlAll(mid, CW_tmd, 0);
//    }
//  }

  while(1){
    trainTrainCtrlAll(mid, dir, mag);
    trainTrainCtrlAllRx(mid, &ec, &magdir);
    printf("response received: ec = %d, magdir = %X\n\r", ec, magdir);
    delay();
    delay();
    mag = ((mag+1) %8);
    if(mag == 0){
      dir ^= 0xF0;
    }
  }
}

void switch_testing(void)
{
  mid_t mid = msgRegister();
  truefalse_e ec;
  char switch_id, switch_state;
  trainSwitchControl(mid, 4, 0);
  trainSwitchCtrlRx(mid, &ec, &switch_id, &switch_state);
  printf("switch: #%d to state %d; ec = %d\n", switch_id, switch_state, ec);
  while(1);
}

/* Name: delay
 * Parameters:
 * Returns:
 * Description: creates a crude delay by counting to 500 500 times
 */
void delay(void)
{
  // generate a crude delay
  int i,j;
  for(i = 0; i < 500; i++){
    for(j = 0; j < 1500; j++);
  }
}
