#ifndef INPUT_H
#define input_H

#pragma once

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>

extern _calculatorState calcState;

bool isSpecialFunctionKey(DWORD keyPressed);
BOOL updateInputMode(DWORD keyPressed);

#endif