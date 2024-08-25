#ifndef INPUT_H
#define INPUT_H

#pragma once

#undef UNICODE
#undef _UNICODE

#include <windows.h>


extern const short int MAX_DIGITS_FOR_BASE[];

BOOL appendDigit(char* accumulatedValue, int digit);
BOOL isClearKey(DWORD keyPressed);
BOOL isNumericInput(DWORD keyPressed);
BOOL isPreviousKeyOperator();
BOOL isOperatorKey(DWORD keyPressed);
BOOL isSpecialFunctionKey(DWORD keyPressed);
BOOL updateInputMode(DWORD keyPressed);

#endif