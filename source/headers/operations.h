/*-----------------------------------------------------------------------------
    operations.h -- Header file for Calculation and Conversion Functions of the
                   Windows Calculator (reconstructed code).

                   This header defines the 80-bit extended precision float
                   structure (`ExtendedFloat80`) and declares function prototypes
                   for the operations.c module.  It provides an interface for
                   other modules to access the calculator's arithmetic, logic,
                   transcendental, and conversion functions.

 -------------------------------------------------------------------------------*/

#ifndef OPERATIONS_H
#define OPERATIONS_H

#pragma once

#include "..//headers//main.h"


 // Maximum allocation size from the heap, likely related to limitations of 
 // the custom memory manager.  This leaves a 4KB "safety margin" below the 4GB address space limit.
#define MAX_HEAP_ALLOCATION_SIZE 0xFFFFD001

// Mask to align memory addresses to 4-byte boundaries.  Used for both 
// signed and unsigned values. 
#define ALIGNMENT_MASK 0xFFFFFFFC

// Unsigned version of the alignment mask (though it doesn't affect the bit pattern).
#define ALIGNMENT_MASK_UNSIGNED 0xFFFFFFFCU


void intToExtendedFloat80(LONGLONG value);
BOOL isValueOverflow(int digit);
BOOL isValueOverflowExtended(void);
void shiftMultiWordInteger(DWORD* highWord, int shiftAmount);
void stringToExtendedFloat80(const char* str);


#endif