#ifndef __WM_H__
#define __WM_H__

#include "data_structures.h"
#include <stdint.h>

// Public Window Structures

// Coordinates for the cursor
typedef struct _cursor_coordinates_t_{
  uint8_t x;
  uint8_t y;
}cursor_t;

typedef enum _WINDOWNAME_E_{ //  ________
  TOP_wn,                    // |________|
  TOPMID_wn,                 // |________|
  BOTMID_wn,                 // |________|
  BOT_wn,                    // |________|
  NONE_wn
}window_name_e;

// Public WM functions
void wm_init(void); // init - set globals to default values, draw up the windows, draw up the prompts, etc
window_name_e wmRequestWindow(void); // returns a window if available, blocks & ContextSwitches else
void wmReleaseWindow(window_name_e wid); // the complement to requestWindow
void wmPuts(char* str, window_name_e wid); // puts a string into a strpack, then on the tx queue if available
void wmGets(char* dst, uint8_t length, window_name_e wid); // strncpy's from the window's UART RX queue,
void wmMoveCursor(uint8_t, uint8_t); // change the master cursor to the requested position
void wmNewline(window_name_e wid); // for printing a newline
void wmSetScroll(window_name_e wid);

#endif
