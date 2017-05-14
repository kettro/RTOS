#ifndef __WINDOW_DATA_STRUCTURES_H__
#define __WINDOW_DATA_STRUCTURES_H__

#define BASE_STR_LENGTH   80
#define NUM_TXQUEUES      16 // must be a power of 2
#define NUM_WINDOWS       4
#define DEFAULT_WINDOWX   80
#define DEFAULT_WINDOWY   10

typedef struct _strpack_t_{
  char str[BASE_STR_LENGTH];
}strpack_t;

typedef struct _strpack_counter_t_{
  volatile uint8_t head; // Top of the queue, next item to be read
  volatile uint8_t tail; // bottom of the queue, last item added.
  volatile uint8_t str; // string index
  volatile uint8_t len; // number of queues occupied
}strpack_cnt_t;

#endif
