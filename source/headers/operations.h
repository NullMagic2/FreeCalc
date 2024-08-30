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

#include ".//headers//main.h"

void intToExtendedFloat80(LONGLONG value);
BOOL isValueOverflow(int digit);
BOOL isValueOverflowExtended(void);
void shiftMultiWordInteger(DWORD* highWord, int shiftAmount);
void stringToExtendedFloat80(const char* str);


#endif