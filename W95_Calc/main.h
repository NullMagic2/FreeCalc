/*-----------------------------------------------------------------------------
    main.h --  Header file for the Windows Calculator reconstruction project.

               This header defines common data types, structures, constants,
               and function prototypes used throughout the application.

               Key elements include:

               - Data Types:  Defines ushort, uint, and enumerations for
                              calculator modes, button states, and error codes.

               - Structures:  Defines structures for application path, code
                              page information, environment variables,
                              calculator windows, and calculator state.

               - Constants: Defines constants for key codes, button IDs,
                            system settings, layout dimensions, and more.

               - Function Prototypes: Declares prototypes for functions used
                                       in window creation, message handling,
                                       UI initialization, and calculator logic.

 -------------------------------------------------------------------------------*/

#ifndef MAIN_H
#define MAIN_H

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>

typedef unsigned short ushort;
typedef unsigned int uint;

#define KEY_RANGE_START   0x56
#define KEY_RANGE_END     0x5f

#define INVALID_BUTTON    0xffffffff

#define HELP_CONTEXT_DATA 0x40c128
#define STAT_BUTTON_ID    0x67
#define STAT_ALT_BUTTON_ID 0x8c
#define MEMORY_BUTTON_START 0x40
#define MEMORY_BUTTON_END 0x47
#define MEMORY_BUTTON_DEFAULT 0x41
#define DIGIT_BUTTON_START 0x2f
#define DIGIT_BUTTON_END  0x3a
#define DIGIT_BUTTON_DEFAULT 0x30
#define MAX_COMMAND_ID    0x79
#define ID_STAT_BUTTON    100
#define ID_STAT_BUTTON_ALT 101
#define ID_EDIT_PASTE     102

#define SYSTEM_CODE_PAGE -3
#define DEFAULT_BACKGROUND_COLOR "C0C0C0"
#define CALCULATOR_APP_NAME "Calculator"

//Scientific mode
#define SCIENTIFIC_CALC_ROWS 7
#define SCIENTIFIC_CALC_COLS 10

//Standard mode
#define STANDARD_CALC_ROWS 6
#define STANDARD_CALC_COLS 5

//Padding and spacing (in pixels)
#define HORIZONTAL_MARGIN 2
#define VERTICAL_MARGIN 1

#define BUTTON_HORIZONTAL_SPACING 2
#define BUTTON_VERTICAL_SPACING 2

//Assumes English locale
#define DEFAULT_DECIMAL_SEPARATOR '.'

typedef enum {
    STANDARD_MODE = 0,
    SCIENTIFIC_MODE = 1
} _calculatorMode;

typedef enum {
    STATE_UP = 0x66,
    STATE_DOWN = 0x65
} _buttonState;

typedef enum {
    ERROR_MEMORY_ALLOCATION = 8,
    ERROR_ENVIRONMENT_INITIALIZATION = 9
    // Add other error codes as needed
} _errorCode;

typedef struct {
    char* fullPath;
    char** components;
    int componentCount;
} _applicationPath;

typedef struct {
    int currentCodePage;
    int codePageSpecificFlag;
    int customCharTypeFlag1;
    int customCharTypeFlag2;
    int customCharTypeFlag3;
} _codePageInfo;

typedef struct {
    char** variables;
    int count;
} _environmentVariables;

typedef struct {
    HWND main;
    HWND scientific;
} _calculatorWindows;

typedef struct {
    const char* className;
    const char* registryKey;
    char helpFilePath[MAX_PATH];
    DWORD currentBackgroundColor;
    DWORD defaultPrecisionValue;
    DWORD errorCodeBase;
    HINSTANCE appInstance;
    DWORD pressedButton;
    HWND windowHandle;
    _calculatorMode mode;
    DWORD currentValueHighPart;
    DWORD accumulatedValue;
    DWORD lastValue;
    int errorState;
    UINT currentKeyPressed;
    BOOL isHighContrastMode;
    char decimalSeparator;
    char decimalSeparatorBuffer[2];
    int buttonHorizontalSpacing;
} _calculatorState;

extern DWORD defaultPrecisionValue;
extern DWORD errorCodeBase;

extern DWORD currentValueHighPart;
extern DWORD accumulatedValue;
extern DWORD lastValue;
extern int errorState;
extern UINT currentKeyPressed;

extern UINT elementStateTable[];

// Function prototypes
void adjustMemoryAllocation(void);
LRESULT CALLBACK calcWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD configureCodePageSettings(int requestedCodePage);
DWORD getCalculatorButton(ushort x, ushort y);
void initCalculatorState(void);
bool initInstance(HINSTANCE appInstance, int windowMode);
void initApplicationCodePage(void);
void initColors(int forceUpdate);
void initStandardStreams(void);
ATOM registerCalcClass(HINSTANCE appInstance);
void handleCalculationError(int errorCode);
bool handleContextHelp(HWND hwnd, HINSTANCE hInstance, UINT param);
void processButtonClick(uint keypressed);
void updateButtonState(uint buttonID, int state);
void updateDisplay(void);
void refreshInterface(void);
void initApplicationPath(void);
void initEnvironmentVariables(void);
DWORD initCalculatorRuntime(int initializationFlags);
void updateDecimalSeparator();

#endif // MAIN_H