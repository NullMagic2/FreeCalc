#ifndef MEMORY_H
#pragma once

#include "..//headers//main.h"

typedef struct memoryBlock {
    struct memoryBlock* nextBlock;    // Pointer to the next memory block in the list
    DWORD flags_and_size;            // Flags indicating the block's state and its size
} memoryBlock, * PmemoryBlock; // Correct the pointer type to 'PmemoryBlock'

extern memoryBlock* memoryBlockListHead; //Corrected typo

DWORD allocateAdditionalMemory(DWORD requestedSize);
DWORD allocateMemoryFromHeap(DWORD requestedSize);


#define MEMORY_H
#endif