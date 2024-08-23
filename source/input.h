#pragma once

#ifndef INPUT_H
#define input_H

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>

bool isSpecialFunctionKey(DWORD keyPressed);
BOOL updateInputMode(DWORD keyPressed);

#endif