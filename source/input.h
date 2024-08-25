#ifndef INPUT_H
#define INPUT_H

#pragma once

#undef UNICODE
#undef _UNICODE

#include "main.h"
#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>


extern const short int MAX_DIGITS_FOR_BASE[];

extern _calculatorState calcState;

BOOL appendDigit(char* accumulatedValue, int digit);
BOOL isClearKey(uint keyPressed);
BOOL isNumericInput(uint keyPressed);
BOOL isPreviousKeyOperator();
BOOL isOperatorKey(uint keyPressed);
bool isSpecialFunctionKey(DWORD keyPressed);
BOOL updateInputMode(DWORD keyPressed);

#endif