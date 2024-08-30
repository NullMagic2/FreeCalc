/*-----------------------------------------------------------------------------
    input.h --  Header file for Input Handling Functions of the Windows
                Calculator (reconstructed code).

                This header defines constants and function prototypes for
                the input.c module. It provides an interface for other modules
                to access input-related functionality.

 -------------------------------------------------------------------------------*/

#ifndef INPUT_H
#define INPUT_H

#pragma once

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include ".//headers//main.h"
#include "operations.h"
#include <float.h>
#include <math.h>
#include <limits.h>


extern const short int MAX_DIGITS_FOR_BASE[];

BOOL appendDigit(char* accumulatedValue, int digit);
int convertKeyToDigit(DWORD keyCode);
BOOL isClearKey(DWORD keyPressed);
BOOL isNumericInput(DWORD keyPressed);
BOOL isPreviousKeyOperator();
BOOL isOperatorKey(DWORD keyPressed);
BOOL isSpecialFunctionKey(DWORD keyPressed);
BOOL updateInputMode(DWORD keyPressed);

#endif