/*
 * File: alloc.c
 * Author: Ross Kettleson
 * Revision: 09/26/16
 */
#include "alloc.h"
#include "alloc_defines.h"

typedef union u_ledger_t{
  uint16_t word[8];
  uint64_t first_four_words;
} ledger_t;

// Local Variables
ledger_t ledger[SIZES_PER_SRAM]; // Store all alloc-able blocks
uint8_t size_count[SIZES_PER_SRAM]; // Store # of blocks left
uint16_t mask[SIZES_PER_SRAM] = { 0x01, 0x03, 0xF, 0xFF, 0xFFFF }; // block mask for setting/clearing

// Functions:

/* Function: alloc()
 * Purpose: To find and allocate a sufficiently sized block of memory
 * Returns: Pointer to the allocated block of memory
 * Arguments: Size request, in bytes: minimum size of desired block
 */
void* alloc(uint16_t request)
{
  // Declare automatics
  uint8_t size, word, bit; // coordinates
  uint16_t* ledger_ptr; // eventual pointer to the ledger entry
  ledger_t* ledger_size_ptr; // temporary pointer to the size entry in the ledger

  // Ensure the request is smaller than the Max Block Size
  if(request > BLOCK_SIZE_MAX){ return NULL; }

  // The size category sorta based on the size: 0x80. 0x100, etc
  // Each successive size is twice the size of the previous one
  for(size = 0; size < SIZES_PER_SRAM; size++){
    // Size 0: request <= 0x80 * 2^0
    // Size 1: request <= 0x80 * 2^1 ...
    if(request <= BYTES_PER_BLOCK << size){ break; }
  }

  // check for free blocks in the size category
  for(;!size_count[size];  size++){
    if(size < SIZES_PER_SRAM){ continue; } // if the size == 0, this skips an if-check
    /* else if(size == _SIZE_MAX) */return NULL; // Ledger Error: Incorrect count
  }

  // find the first word with a free spot
  // check if the first 4 words are allocated: if so, default the word index to 4
  ledger_size_ptr = &(ledger[size]);
  word = (ledger_size_ptr->first_four_words < SIXTY_FOUR_BITS) ? 0 : 4;
  for(; ledger_size_ptr->word[word] == 0xFFFF; word++){
    if(word == WORDS_PER_SIZE - 1){ return NULL; }
  }

  // Find the first free bit- loop 0/4/8/C -> BITMAX
  ledger_ptr = &(ledger_size_ptr->word[word]); // Save the finalized location as a pointer - faster than using the struct
  // check the value of the word, if >1 byte, can start there
  // costs an extra 8 cycles per check, but caps the cycle increase to 4 checks
  bit = (*ledger_ptr & 0xF)<0xF ? 0 : (*ledger_ptr & 0xFF)<0xFF ? 4 : (*ledger_ptr & 0xFFF)<0xFFF ? 8 : 12;
  for(;*ledger_ptr & (1<<bit); bit++){
    if(bit >= BITS_PER_WORD - 1){ return NULL; } // ledger error: incorrect count kept
  }

  // update the ledger
  *ledger_ptr |= mask[size] << bit;
  size_count[size]--; // Update the count

  // Generate the pointer from the coordinates
  uint16_t index = (size << BLOCKS_PER_SIZE_SHIFT) + (word << BLOCKS_PER_WORD_SHIFT) + bit; // 1<<7 = blocks/size, 1<<4 = blocks/word
  uint32_t offset = index << BLOCKS_PER_SIZE_SHIFT; // 1<<7 = blocks in a size

  // Return the pointer: the offset + the memory start
  return (void*)((char*)SYS_MEM_START + offset);
}

/* Function: dealloc()
 * Purpose: To free memory previously allocated, now no longer in use. 
 * Returns: none
 * Arguments: A pointer the the start of a block of memory. 
 */
void dealloc(void* pointer)
{
  uint16_t index;
  uint8_t size, word, bit; // coordinates
  uint16_t* ledger_ptr;
  // calculate the index from the pointer
  index = (uint16_t)(((uintptr_t)pointer - SYS_MEM_START) >> BLOCKS_PER_SIZE_SHIFT); // 1<<7 = blocks/size

  // calculate the coordinates:
  // => Modulo is emulated by: x % 2^n == x & (2^n - 1)
  size = index >> BLOCKS_PER_SIZE_SHIFT;
  word = (index & (BLOCKS_PER_SIZE - 1)) >> BLOCKS_PER_WORD_SHIFT;
  bit = (index & BLOCKS_PER_WORD - 1);

  ledger_ptr = &(ledger[size].word[word]);

  // Check if bit is set:
  if(!(*ledger_ptr & (1<<bit))){ return; } // is free

  // Mark as free, update the ledger
  *ledger_ptr &= ~(mask[size] << bit);
  size_count[size]++;

  pointer = NULL;
}

/* Function: alloc_init()
 * Purpose: To initialize the number of blocks per size category
 * Returns: none
 * Arguments: none
 */
void alloc_init(void)
{
  // Initialisation code for the allocation utility;
  // to be run by the OS at startup.
  int i;
  // Each larger Size is twice as big as the smaller
  for(i = 0; i < SIZES_PER_SRAM; i++){
    size_count[i] = (BLOCKS_PER_SIZE >> i);
  }
}
