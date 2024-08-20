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

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>

typedef unsigned short ushort;
typedef unsigned int uint;

//Do-nothing instruction, as a placeholder.
#define pass (void)0

#define KEY_RANGE_START   0x56
#define KEY_RANGE_END     0x5f

#define INVALID_BUTTON    0xffffffff  // Represents an invalid or unassigned button ID, using max DWORD value
#define HELP_CONTEXT_DATA 0x40c128    // Pointer to help context data structure, likely used with WinHelp API

#define STAT_BUTTON_ID    0x67        // ID for the statistics button in standard mode (103 in decimal)
#define STAT_ALT_BUTTON_ID 0x8c       // Alternate ID for the statistics button in scientific mode (140 in decimal)

#define MEMORY_BUTTON_START 0x40      // Start of memory button ID range (64 in decimal)
#define MEMORY_BUTTON_END 0x47        // End of memory button ID range (71 in decimal)
#define MEMORY_BUTTON_DEFAULT 0x41    // Default memory button ID, likely "Memory Recall" (65 in decimal)

#define DIGIT_BUTTON_START 0x2f       // Start of digit button ID range (47 in decimal)
#define DIGIT_BUTTON_END  0x3a        // End of digit button ID range (58 in decimal)
#define DIGIT_BUTTON_DEFAULT 0x30     // Default digit button ID, likely "0" (48 in decimal)

#define MAX_COMMAND_ID    0x79        // Maximum command ID (121 in decimal) for calculator buttons (main.c)
                                      // Used to validate input in WM_COMMAND processing
                                      // Allows treating WM_COMMAND and WM_CHAR messages similarly

#define ID_STAT_BUTTON    100         // Resource ID for statistics button, used in dialog resources
#define ID_STAT_BUTTON_ALT 101        // Alternate resource ID for statistics button, used in dialog resources
#define ID_EDIT_PASTE     102         // Resource ID for paste command in edit menu, used in menu resources

#define SYSTEM_CODE_PAGE -3
#define DEFAULT_BACKGROUND_COLOR "C0C0C0"
#define CALCULATOR_APP_NAME "Calculator"

//Scientific mode
#define SCIENTIFIC_CALC_ROWS 7
#define SCIENTIFIC_CALC_COLS 10

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

// Calculation result codes
#define STATUS_SUCCESS                0  // Calculation completed successfully
#define STATUS_DIVISION_BY_ZERO       1  // Cannot divide by zero
#define STATUS_CLIPBOARD_ERROR        2  // Cannot open Clipboard
#define STATUS_INSUFFICIENT_MEMORY    3  // Insufficient memory for data
#define STATUS_INVALID_INPUT          4  // Invalid input for function
#define STATUS_OVERFLOW               5  // Result is too large for display
#define STATUS_UNDERFLOW              6  // Result is too small for display
#define STATUS_UNDEFINED_RESULT       7  // Result of function is undefined

static const struct {
    int status;
    const char* message;
} STATUS_MESSAGE_TABLE[] = {
    {STATUS_DIVISION_BY_ZERO, "Cannot divide by zero"},
    {STATUS_CLIPBOARD_ERROR, "Cannot open Clipboard."},
    {STATUS_INSUFFICIENT_MEMORY, "Insufficient memory for data; close one or more Windows Applications to increase available memory."},
    {STATUS_INVALID_INPUT, "Invalid input for function"},
    {STATUS_OVERFLOW, "Result is too large for display"},
    {STATUS_UNDERFLOW, "Result is too small for display"},
    {STATUS_UNDEFINED_RESULT, "Result of function is undefined"}
};

#define STATUS_MESSAGE_TABLE_END (STATUS_MESSAGE_TABLE + sizeof(STATUS_MESSAGE_TABLE)/sizeof(STATUS_MESSAGE_TABLE[0]))

//Calculator buttons – standard mode
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

#define IDC_RADIO_DEG     0xAD  // Degrees
#define IDC_RADIO_RAD     0xAE  // Radians
#define IDC_RADIO_GRAD    0xAF  // Gradians

#define IDC_RADIO_HEX     0xB0  // Hexadecimal
#define IDC_RADIO_DEC     0xB1  // Decimal
#define IDC_RADIO_OCT     0xB2  // Octal
#define IDC_RADIO_BIN     0xB3  // Binary

#define IDC_BUTTON_AND    0xB8  // Bitwise AND
#define IDC_BUTTON_OR     0xB9  // Bitwise OR
#define IDC_BUTTON_XOR    0xBA  // Bitwise XOR
#define IDC_BUTTON_NOT    0xBB  // Bitwise NOT
#define IDC_BUTTON_LSH    0xBC  // Left shift

// Display controls
#define IDC_EDIT_RESULT   0x9B  // Result display
#define IDC_EDIT_EXPR     0xBE  // Expression display


// Display control
#define IDC_EDIT_RESULT  0x9B

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
    const char* modeText[2];
    DWORD currentValueHighPart;
    DWORD accumulatedValue;
    DWORD lastValue;
    DWORD memoryRegister[2];
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
const char* getStatusCode(int statusCode);
void initCalculatorState(void);
bool initInstance(HINSTANCE appInstance, int windowMode);
void initApplicationCodePage(void);
void initApplicationPath(void);
void initColors(int forceUpdate);
void initStandardStreams(void);
void initEnvironmentVariables(void);
ATOM registerCalcClass(HINSTANCE appInstance);
void handleCalculationError(int errorCode);
bool handleContextHelp(HWND hwnd, HINSTANCE hInstance, UINT param);
void processButtonClick(uint keypressed);
void updateButtonState(uint buttonID, int state);
void updateDisplay(void);
void refreshInterface(void);

DWORD initCalculatorRuntime(int initializationFlags);
void updateDecimalSeparator();

#endif // MAIN_H