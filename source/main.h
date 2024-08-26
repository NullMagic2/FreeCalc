/*-----------------------------------------------------------------------------
    main.h --  Header file for the Windows Calculator reconstruction project (reconstructed code).

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

#pragma once

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>
#include "input.h"

typedef unsigned short ushort;
typedef unsigned int uint;

extern int BUTTON_BASE_SIZE;

//Do-nothing instruction, as a placeholder.
#define pass (void)0

#define KEY_RANGE_START   0x56
#define KEY_RANGE_END     0x5f

#define INVALID_BUTTON    0xffffffff  // Represents an invalid or unassigned button ID, using max DWORD value
#define HELP_CONTEXT_DATA 0x40c128    // Pointer to help context data structure, likely used with WinHelp API

#define MEMORY_BUTTON_START 0x40      // Start of memory button ID range (64 in decimal)
#define MEMORY_BUTTON_END 0x47        // End of memory button ID range (71 in decimal)
#define MEMORY_BUTTON_DEFAULT 0x41    // Default memory button ID, likely "Memory Recall" (65 in decimal)

#define DIGIT_BUTTON_START 0x2f       // Start of digit button ID range (47 in decimal)
#define DIGIT_BUTTON_END  0x3a        // End of digit button ID range (58 in decimal)
#define DIGIT_BUTTON_DEFAULT 0x30     // Default digit button ID, likely "0" (48 in decimal)

#define HIGH_CONTRAST_MODE_FLAG 0x40c064 // Flag that checks for high contrast mode flag.

#define MAX_COMMAND_ID    0x79        // Maximum command ID (121 in decimal) for calculator buttons (main.c)
                                      // Used to validate input in WM_COMMAND processing
                                      // Allows treating WM_COMMAND and WM_CHAR messages similarly

#define ID_EDIT_PASTE     102         // Resource ID for paste command in edit menu, used in menu resources

#define SYSTEM_CODE_PAGE -3
#define DEFAULT_BACKGROUND_COLOR "C0C0C0"
#define CALCULATOR_APP_NAME "Calculator"

//Scientific mode
#define SCIENTIFIC_CALC_ROWS 7
#define SCIENTIFIC_CALC_COLS 10
#define SCIENTIFIC_BUTTON_EXTRA_WIDTH 8

//Standard mode
#define STANDARD_CALC_ROWS 6
#define STANDARD_CALC_COLS 5
#define IDC_TEXT_STANDARD_MODE  0x19D  // Adjust this value based on your actual resource definitions
#define IDC_TEXT_SCIENTIFIC_MODE 0x19E // Adjust this value based on your actual resource definitions

//Padding and spacing (in pixels)
#define HORIZONTAL_MARGIN 2
#define VERTICAL_MARGIN 1

#define BUTTON_HORIZONTAL_SPACING 2
#define BUTTON_VERTICAL_SPACING 2

#define ERROR_STATE_ACTIVE 1
#define ERROR_STATE_INACTIVE 0

//Calculation status codes
#define STATUS_READY    0  // Ready to do a calculation
#define STATUS_WORKING  1  // Working on a calculation
#define STATUS_DONE     2  // Done with a calculation

#define INITIAL_MEMORY_SIZE 0x4000  // Initial memory allocation size (16 KB)
#define EXTENDED_MEMORY_SIZE 0x40000 // Memory allocation size for higher precision (256 KB) 


// Calculation result codes
#define STATUS_SUCCESS                0  // Calculation completed successfully
#define STATUS_DIVISION_BY_ZERO       1  // Cannot divide by zero
#define STATUS_CLIPBOARD_ERROR        2  // Cannot open Clipboard
#define STATUS_INSUFFICIENT_MEMORY    3  // Insufficient memory for data
#define STATUS_INVALID_INPUT          4  // Invalid input for function
#define STATUS_OVERFLOW               5  // Result is too large for display
#define STATUS_UNDERFLOW              6  // Result is too small for display
#define STATUS_UNDEFINED_RESULT       7  // Result of function is undefined

//Maximum number of digits the calculator can handle.
#define MAX_BINARY_DIGITS 32
#define MAX_OCTAL_DIGITS 11
#define MAX_HEXADECIMAL_DIGITS 8

#define MAX_DECIMAL_DIGITS 13      // Decimal mode. For hex mode, this is 8.
#define MAX_FRACTIONAL_DIGITS 28   // Fractional part.

#define MAX_DISPLAY_DIGITS 35      // Maximum number of digits the calculator can display.
#define MAX_STANDARD_PRECISION 12  // Maximum precision for standard mode (32 bits)

#define IDM_VIEW_STANDARD               0x9C4E  // Command to switch to standard calculator view

static const char* STATUS_MESSAGE_TABLE[] = {
    "Success",
    "Cannot divide by zero",
    "Cannot open Clipboard.",
    "Insufficient memory for data; close one or more Windows Applications to increase available memory.",
    "Invalid input for function.",
    "Result is too large for display.",
    "Result is too small for display.",
    "Result of function is undefined."
};

#define STATUS_MESSAGE_TABLE_END (STATUS_MESSAGE_TABLE + sizeof(STATUS_MESSAGE_TABLE)/sizeof(STATUS_MESSAGE_TABLE[0]))

//Calculator buttons � standard mode
// Standard Calculator Button IDs
#define IDC_BUTTON_MC     0x80  // Memory Clear
#define IDC_BUTTON_MR     0x81  // Memory Recall
#define IDC_BUTTON_MS     0x82  // Memory Store
#define IDC_BUTTON_MPLUS  0x83  // Memory Add

#define IDC_BUTTON_BACK   0x84  // Backspace
#define IDC_BUTTON_CE     0x85  // Clear Entry
#define IDC_BUTTON_C      0x86  // Clear All

#define IDC_BUTTON_7      0x87  // Digit 7
#define IDC_BUTTON_8      0x88  // Digit 8
#define IDC_BUTTON_9      0x89  // Digit 9
#define IDC_BUTTON_DIV    0x8A  // Division

#define IDC_BUTTON_4      0x8B  // Digit 4
#define IDC_BUTTON_5      0x8C  // Digit 5
#define IDC_BUTTON_6      0x8D  // Digit 6
#define IDC_BUTTON_MUL    0x8E  // Multiplication

#define IDC_BUTTON_1      0x8F  // Digit 1
#define IDC_BUTTON_2      0x90  // Digit 2
#define IDC_BUTTON_3      0x91  // Digit 3
#define IDC_BUTTON_SUB    0x92  // Subtraction

#define IDC_BUTTON_0      0x93  // Digit 0
#define IDC_BUTTON_DOT    0x94  // Decimal point
#define IDC_BUTTON_EQ     0x95  // Equals
#define IDC_BUTTON_ADD    0x96  // Addition

#define IDC_BUTTON_SQRT   0x97  // Square root
#define IDC_BUTTON_PERC   0x98  // Percentage
#define IDC_BUTTON_INV    0x99  // Reciprocal (1/x)
#define IDC_BUTTON_NEG    0x9A  // Negate (+/-)

// Scientific Calculator Additional Button IDs
#define IDC_BUTTON_SIN    0x9C  // Sine
#define IDC_BUTTON_COS    0x9D  // Cosine
#define IDC_BUTTON_TAN    0x9E  // Tangent

#define IDC_BUTTON_ASIN   0x9F  // Arc sine
#define IDC_BUTTON_ACOS   0xA0  // Arc cosine
#define IDC_BUTTON_ATAN   0xA1  // Arc tangent

#define IDC_BUTTON_LOG    0xA2  // Logarithm (base 10)
#define IDC_BUTTON_LN     0xA3  // Natural logarithm

#define IDC_BUTTON_EXP    0xA4  // Exponential (e^x)
#define IDC_BUTTON_XY     0xA5  // x to the power of y

#define IDC_BUTTON_PI     0xA6  // Pi constant

#define IDC_BUTTON_LPAR   0xA7  // Left parenthesis
#define IDC_BUTTON_RPAR   0xA8  // Right parenthesis

#define IDC_BUTTON_SQR    0xA9  // x squared
#define IDC_BUTTON_CUBE   0xAA  // x cubed
#define IDC_BUTTON_FACT   0xAB  // Factorial

#define IDC_BUTTON_MSUB   0xAC  // Memory Subtract
#define IDC_BUTTON_MOD    0xAD  // Modulo

#define IDC_RADIO_DEG     0xAE  // Degrees
#define IDC_RADIO_RAD     0xAF  // Radians
#define IDC_RADIO_GRAD    0xB0  // Gradians

#define IDC_RADIO_HEX     0xB1  // Hexadecimal
#define IDC_RADIO_DEC     0xB2  // Decimal
#define IDC_RADIO_OCT     0xB3  // Octal
#define IDC_RADIO_BIN     0xB4  // Binary

#define IDC_BUTTON_AND    0xB8  // Bitwise AND
#define IDC_BUTTON_OR     0xB9  // Bitwise OR
#define IDC_BUTTON_XOR    0xBA  // Bitwise XOR
#define IDC_BUTTON_NOT    0xBB  // Bitwise NOT
#define IDC_BUTTON_LSH    0xBC  // Left shift

#define IDC_BUTTON_STA 0x8c           // Statistics button in scientific mode

// Buttons within the statistics window
#define IDC_BUTTON_STAT_RED  0x75    // RET (Retrieve) button
#define IDC_BUTTON_STAT_LOAD 0x76    // LOAD button
#define IDC_BUTTON_STAT_CE   0x77    // C (Clear Entry) button
#define IDC_BUTTON_STAT_CAD  0x78    // CAD (Clear All Data) button

// Hexadecimal digits
#define IDC_BUTTON_A      0xB5  // Hexadecimal digit A
#define IDC_BUTTON_B      0xB6  // Hexadecimal digit B
#define IDC_BUTTON_C      0xB7  // Hexadecimal digit C
#define IDC_BUTTON_D      0xBD  // Hexadecimal digit D
#define IDC_BUTTON_E      0xBE  // Hexadecimal digit E
#define IDC_BUTTON_F      0xBF  // Hexadecimal digit F


// Other constants
#define IDC_BUTTON_DECIMAL 0x55
#define IDC_BUTTON_RPAREN  0x29
#define IDC_BUTTON_EXP     0x12D
#define IDC_BUTTON_INV     0x7D

// Display controls
#define IDC_EDIT_RESULT   0x9B  // Result display
#define IDC_EDIT_EXPR     0xBE  // Expression display


// Display control
#define IDC_EDIT_RESULT  0x9B

//Assumes English locale
#define DEFAULT_DECIMAL_SEPARATOR '.'

// Structure to represent an 80-bit extended precision floating-point number
typedef struct {
    unsigned short exponent;      // 15 bits: exponent + sign 
    unsigned int mantissaLow;    // 32 bits: lower part of mantissa
    unsigned int mantissaHigh;   // 32 bits: upper part of mantissa
} _extendedFloat80;


typedef enum {
    STANDARD_MODE = 0,
    SCIENTIFIC_MODE = 1,
    SCIENTIFIC_NOTATION = 2,
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
    char accumulatedValue[MAX_DISPLAY_DIGITS];  // Current value or result of the last operation
    HINSTANCE appInstance;                      // Handle to the current instance of the application
    int buttonHorizontalSpacing;                // Horizontal spacing between calculator buttons
    const char* className;                      // Name of the window class for the calculator
    int currentSign;                            // Positive / negative sign of the current input.
    DWORD currentOperator;                      // Current operation (ADDITION, SUBTRACTION, MULTIPLICATION...)
    int currentPrecisionLevel;                  // Initialize to max standard precision
    DWORD currentValueHighPart;                 // High part of the current value (for high precision)
    DWORD currentBackgroundColor;               // Current background color of the calculator
    char decimalSeparator;                      // Character used as decimal separator
    char decimalSeparatorBuffer[2];             // Buffer for storing decimal separator
    DWORD defaultPrecisionValue;                // Default precision for calculations
    int errorState;                             // Current error state of the calculator
    DWORD errorCodeBase;                        // Base value for error codes
    BOOL hasOperatorPending;                    // Flag indicating if an operator is pending
    BOOL isHighContrastMode;                    // Flag indicating if high contrast mode is active
    BOOL isInputModeActive;                     // Flag indicating if input mode is active
    BOOL isScientificModeActive;                // Flag indicating if scientific mode is active
    char helpFilePath[MAX_PATH];                // Path to the calculator's help file
    DWORD keyPressed;                           // Stores the currently pressed key
    DWORD lastValue;                            // Previous value before the last operation
    _calculatorMode mode;                       // Current mode of the calculator (Standard or Scientific)
    const char* modeText[2];                    // Text representations of calculator modes
    DWORD memoryRegister[2];                    // Memory storage for calculator operations
    int numberBase;                             // Current number base (2 for binary, 8 for octal, 10 for decimal, 16 for hexadecimal)
    HWND statisticsWindow;                      // Handle to the statistics window
    BOOL statisticsWindowOpen;                  // Flag to track if the statistics window is open
    const char* registryKey;                    // Registry key for storing calculator settings
    _extendedFloat80 scientificNumber;          // 80-bit extended precision floating-point number
    HWND scientificWindowHandle;                // Handle to the scientific calculator window
    HWND windowHandle;                          // Handle to the main calculator window
    
} _calculatorState;

extern _calculatorState calcState;


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
const char* getStatusCode(int statusCode);
void initCalcState(void);
bool initInstance(HINSTANCE appInstance, int windowMode);
void initApplicationCodePage(void);
void initApplicationPath(void);
DWORD initCalcRuntime(int initializationFlags);
void initColors(int forceUpdate);
void initStandardStreams(void);
void initEnvironmentVariables(void);
void handleCalculationError(int errorCode);
bool handleContextHelp(HWND hwnd, HINSTANCE hInstance, UINT param);
void processButtonClick(uint currentKeyPressed);
void refreshInterface(void);
ATOM registerCalcClass(HINSTANCE appInstance);
void resetCalculatorState(void);
void toggleScientificMode(void);
BOOL CALLBACK statisticsWindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ScientificDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void updateButtonState(uint buttonID, int state);
void updateDecimalSeparator();
void updateDisplay(void);

#endif // MAIN_H