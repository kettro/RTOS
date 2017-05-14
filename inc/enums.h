#ifndef __ENUMS_H__
#define __ENUMS_H_

typedef enum _TRUEFALSE_E_{
  FALSE = 0,
  NO = 0,
  TRUE = 1,
  YES = 1
}truefalse_e;

typedef enum _MGMTCALLS_E_{
  CONTEXTSWITCH_mc,
  GETID_mc,
  GETPLEVEL_mc,
  NICE_mc,
  TERM_mc,
  SEND_mc,
  RECEIVE_mc,
  REGISTER_mc,
  CHECKMSGS_mc
}mgmt_calls_e;

typedef enum _MGMTRETVALS_E_{
  E_NO_CURRENT_PROCESS_mrv,
  E_INVALID_SENDER_OR_RECEIVER_mrv,
  GOOD_mrv
}mgmt_retvals_e;

typedef enum _PRIORITY_LEVELS_E_{
  TOP_pl,
  ZERO_pl = 0,
  ONE_pl,
  TWO_pl,
  THREE_pl,
  BOTTOM_pl = 3,
  IDLE_pl
}priority_e;

#endif
