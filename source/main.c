/*-----------------------------------------------------------------------------
   main.c --  Main program module for the Windows Calculator (reconstructed code).

               This module handles the core initialization of the application,
               including:

               - Registering the main window class.
               - Creating the main calculator window.
               - Initializing the calculator's state and UI elements.
               - Managing the application's main message loop.

               Key functions include:

               - WinMain: Entry point for the application.
               - registerCalcClass: Registers the main window class.
               - initInstance: Creates the main calculator window.
               - initColors: Initializes colors and UI element visibility based
                             on calculator mode and system settings. 
               - calcWindowProc: Main window procedure for handling messages.

  -----------------------------------------------------------------------------*/

#include ".//headers//main.h"

_calculatorWindows calcWindows = {
    .main = NULL,
    .scientific = NULL
};

_calculatorState calcState;
_calculatorMode calcMode = STANDARD_MODE;

//Default streams and flags
_streams streams;

_environmentVariables envVariables = {
    .variables = NULL,
    .count = 0
};

//This array is essential for categorizing individual characters(bytes) based on their properties within different code pages.Each byte of this array represents a character code(0 - 255).You will use bit flags to indicate the character's properties. For example:
// - Bit 0: Numeric digit(0 - 9)
// - Bit 1 : Uppercase letter(A - Z)
// - Bit 2 : Lowercase letter(a - z)
// - Bit 3 : Lead byte in a double - byte character set(DBCS)
// - Bit 4 : Valid character for hexadecimal input(0 - 9, A - F)
typedef struct {
    BYTE start;   // Starting character code of the range
    BYTE end;     // Ending character code of the range
    BYTE flags;   // Bit flags representing the character properties for the range
} CharRange;

DWORD supportedCodepages[NUM_SUPPORTED_CODEPAGES] = {
    1252, // ANSI (Western European) - This is essential for English and many European languages
    932,  // Japanese Shift-JIS 
    936,  // Simplified Chinese GBK
    949,  // Korean
    950,  // Traditional Chinese Big5 
    850   // OEM (MS-DOS Latin US) - Useful for compatibility with older applications or files
};

typedef struct {
    BYTE start;   // Starting character code of the range
    BYTE end;     // Ending character code of the range
    BYTE flags;   // Bit flags representing the character properties for the range
} CharRange;

CharRange charRangeTable[NUM_SUPPORTED_CODEPAGES * 6];  // 6 ranges per code page

BYTE charTypeFlags[256] = { 0 }; // Initialize all elements to 0
BOOL isCustomCodePage = FALSE;  // Initially set to FALSE (system-determined)

//Stores if a button is visible or not by turning on turning the highest bit of that button on or off.
//To toggle them, XOR the button code against the mask 0x8000.
WORD windowStateTable[] = {
    0x00 | IDC_EDIT_RESULT,        // Standard Mode | Result display
    0x80 | IDC_EDIT_EXPR,          // Scientific Mode | Expression display

    // Memory buttons (both modes)
    0x09 | IDC_BUTTON_MC,          // Both Modes | Memory Clear
    0x09 | IDC_BUTTON_MR,          // Both Modes | Memory Recall
    0x09 | IDC_BUTTON_MS,          // Both Modes | Memory Store
    0x09 | IDC_BUTTON_MPLUS,       // Both Modes | Memory Add
    0x80 | IDC_BUTTON_MSUB,        // Scientific Mode | Memory Subtract

    // Standard mode buttons
    0x00 | IDC_BUTTON_BACK,        // Standard Mode | Backspace
    0x00 | IDC_BUTTON_CE,          // Standard Mode | Clear Entry
    0x00 | IDC_BUTTON_CA,          // Standard Mode | Clear All

    0x00 | IDC_BUTTON_7,           // Standard Mode | Digit 7
    0x00 | IDC_BUTTON_8,           // Standard Mode | Digit 8
    0x00 | IDC_BUTTON_9,           // Standard Mode | Digit 9
    0x00 | IDC_BUTTON_DIV,         // Standard Mode | Division

    0x00 | IDC_BUTTON_4,           // Standard Mode | Digit 4
    0x00 | IDC_BUTTON_5,           // Standard Mode | Digit 5
    0x00 | IDC_BUTTON_6,           // Standard Mode | Digit 6
    0x00 | IDC_BUTTON_MUL,         // Standard Mode | Multiplication

    0x00 | IDC_BUTTON_1,           // Standard Mode | Digit 1
    0x00 | IDC_BUTTON_2,           // Standard Mode | Digit 2
    0x00 | IDC_BUTTON_3,           // Standard Mode | Digit 3
    0x00 | IDC_BUTTON_SUB,         // Standard Mode | Subtraction

    0x00 | IDC_BUTTON_0,           // Standard Mode | Digit 0
    0x00 | IDC_BUTTON_DOT,         // Standard Mode | Decimal point
    0x00 | IDC_BUTTON_EQ,          // Standard Mode | Equals
    0x00 | IDC_BUTTON_ADD,         // Standard Mode | Addition

    0x00 | IDC_BUTTON_SQRT,        // Standard Mode | Square root
    0x00 | IDC_BUTTON_PERC,        // Standard Mode | Percentage
    0x00 | IDC_BUTTON_INV,         // Standard Mode | Reciprocal (1/x)
    0x00 | IDC_BUTTON_NEG,         // Standard Mode | Negate (+/-)

    // Scientific mode buttons
    0x80 | IDC_BUTTON_SIN,         // Scientific Mode | Sine
    0x80 | IDC_BUTTON_COS,         // Scientific Mode | Cosine
    0x80 | IDC_BUTTON_TAN,         // Scientific Mode | Tangent

    0x80 | IDC_BUTTON_ASIN,        // Scientific Mode | Arc sine
    0x80 | IDC_BUTTON_ACOS,        // Scientific Mode | Arc cosine
    0x80 | IDC_BUTTON_ATAN,        // Scientific Mode | Arc tangent

    0x80 | IDC_BUTTON_LOG,         // Scientific Mode | Logarithm (base 10)
    0x80 | IDC_BUTTON_LN,          // Scientific Mode | Natural logarithm

    0x80 | IDC_BUTTON_EXP,         // Scientific Mode | Exponential (e^x)
    0x80 | IDC_BUTTON_XY,          // Scientific Mode | x to the power of y

    0x80 | IDC_BUTTON_PI,          // Scientific Mode | Pi constant

    0x80 | IDC_BUTTON_LPAR,        // Scientific Mode | Left parenthesis
    0x80 | IDC_BUTTON_RPAR,        // Scientific Mode | Right parenthesis

    0x80 | IDC_BUTTON_SQR,         // Scientific Mode | x squared
    0x80 | IDC_BUTTON_CUBE,        // Scientific Mode | x cubed
    0x80 | IDC_BUTTON_FACT,        // Scientific Mode | Factorial

    0x80 | IDC_RADIO_DEG,          // Scientific Mode | Degrees
    0x80 | IDC_RADIO_RAD,          // Scientific Mode | Radians
    0x80 | IDC_RADIO_GRAD,         // Scientific Mode | Gradians

    0x80 | IDC_RADIO_HEX,          // Scientific Mode | Hexadecimal
    0x80 | IDC_RADIO_DEC,          // Scientific Mode | Decimal
    0x80 | IDC_RADIO_OCT,          // Scientific Mode | Octal
    0x80 | IDC_RADIO_BIN,          // Scientific Mode | Binary

    0x80 | IDC_BUTTON_AND,         // Scientific Mode | Bitwise AND
    0x80 | IDC_BUTTON_OR,          // Scientific Mode | Bitwise OR
    0x80 | IDC_BUTTON_XOR,         // Scientific Mode | Bitwise XOR
    0x80 | IDC_BUTTON_NOT,         // Scientific Mode | Bitwise NOT
    0x80 | IDC_BUTTON_LSH,         // Scientific Mode | Left shift
};

// BUTTON_BASE_SIZE: Defines the base size for calculator buttons in pixels.
// It's calculated by mapping dialog units to pixels using MapDialogRect.
// The initial dialog rectangle is set to 24x18 units (right=24, bottom=18).
// After mapping, BUTTON_BASE_SIZE is set to the resulting 'right' value.
// This ensures consistent button sizing across different display configurations.
int BUTTON_BASE_SIZE = 0;

CharRange charRangeTable[NUM_SUPPORTED_CODEPAGES * 6] = {
    // Code Page 1252 (Latin 1 - Western European)
    {'0', '9', CHAR_NUMERIC | CHAR_HEXDIGIT},
    {'A', 'Z', CHAR_UPPERCASE | CHAR_HEXDIGIT},
    {'a', 'z', CHAR_LOWERCASE},
    { 0,    0,    0 },  // No more ranges needed
    { 0,    0,    0 },
    { 0,    0,    0 },

    // Code Page 932 (Japanese Shift-JIS)
    {'0', '9', CHAR_NUMERIC | CHAR_HEXDIGIT},
    {'A', 'Z', CHAR_UPPERCASE | CHAR_HEXDIGIT},
    {'a', 'z', CHAR_LOWERCASE},
    {0x81, 0x9F, CHAR_LEADBYTE},  // Lead byte range 1
    {0xE0, 0xFC, CHAR_LEADBYTE},  // Lead byte range 2
    {0xA1, 0xDF, CHAR_LEADBYTE}, // Lead byte range for half-width katakana

    // Code Page 936 (Simplified Chinese GBK)
    {'0', '9', CHAR_NUMERIC | CHAR_HEXDIGIT},
    {'A', 'Z', CHAR_UPPERCASE | CHAR_HEXDIGIT},
    {'a', 'z', CHAR_LOWERCASE},
    {0xA1, 0xFE, CHAR_LEADBYTE}, // Lead byte range
    { 0,    0,    0 }, // More ranges might be needed for valid GBK characters
    { 0,    0,    0 },

    // Code Page 949 (Korean)
    {'0', '9', CHAR_NUMERIC | CHAR_HEXDIGIT},
    {'A', 'Z', CHAR_UPPERCASE | CHAR_HEXDIGIT},
    {'a', 'z', CHAR_LOWERCASE},
    {0x81, 0xFE, CHAR_LEADBYTE}, // Lead byte range (approximate, check specific Korean mapping)
    { 0,    0,    0 }, // More ranges might be needed for valid Korean characters
    { 0,    0,    0 },

    // Code Page 950 (Traditional Chinese Big5)
    {'0', '9', CHAR_NUMERIC | CHAR_HEXDIGIT},
    {'A', 'Z', CHAR_UPPERCASE | CHAR_HEXDIGIT},
    {'a', 'z', CHAR_LOWERCASE},
    {0xA1, 0xFE, CHAR_LEADBYTE}, // Lead byte range (approximate, check specific Big5 mapping)
    { 0,    0,    0 }, // More ranges might be needed for valid Big5 characters
    { 0,    0,    0 },

    // Code Page 850 (MS-DOS Latin US) 
    {'0', '9', CHAR_NUMERIC | CHAR_HEXDIGIT},
    {'A', 'Z', CHAR_UPPERCASE | CHAR_HEXDIGIT},
    {'a', 'z', CHAR_LOWERCASE},
    { 0,    0,    0 },  // No more ranges needed for this code page (mostly for compatibility)
    { 0,    0,    0 },
    { 0,    0,    0 }
};

DWORD BUTTON_ID_MAP_STANDARD[] = {
    IDC_BUTTON_MC,  IDC_BUTTON_7,  IDC_BUTTON_8,  IDC_BUTTON_9,  IDC_BUTTON_DIV, IDC_BUTTON_SQRT,
    IDC_BUTTON_MR,  IDC_BUTTON_4,  IDC_BUTTON_5,  IDC_BUTTON_6,  IDC_BUTTON_MUL, IDC_BUTTON_PERC,
    IDC_BUTTON_MS,  IDC_BUTTON_1,  IDC_BUTTON_2,  IDC_BUTTON_3,  IDC_BUTTON_SUB, IDC_BUTTON_INV,
    IDC_BUTTON_MPLUS, IDC_BUTTON_0, IDC_BUTTON_NEG, IDC_BUTTON_DOT, IDC_BUTTON_ADD, IDC_BUTTON_EQ,
    IDC_BUTTON_BACK, IDC_BUTTON_CE, IDC_BUTTON_CA
};

DWORD BUTTON_ID_MAP_SCIENTIFIC[] = {
    IDC_BUTTON_MC, IDC_BUTTON_7,  IDC_BUTTON_8,  IDC_BUTTON_9,  IDC_BUTTON_DIV, IDC_BUTTON_MOD,   IDC_BUTTON_AND,
    IDC_BUTTON_MR, IDC_BUTTON_4,  IDC_BUTTON_5,  IDC_BUTTON_6,  IDC_BUTTON_MUL, IDC_BUTTON_OR,    IDC_BUTTON_XOR,
    IDC_BUTTON_MS, IDC_BUTTON_1,  IDC_BUTTON_2,  IDC_BUTTON_3,  IDC_BUTTON_SUB, IDC_BUTTON_LSH,   IDC_BUTTON_NOT,
    IDC_BUTTON_MPLUS, IDC_BUTTON_0, IDC_BUTTON_NEG, IDC_BUTTON_DOT, IDC_BUTTON_ADD, IDC_BUTTON_EQ,  IDC_BUTTON_INT,
    IDC_BUTTON_STA, IDC_BUTTON_F_E,IDC_BUTTON_LPAR,IDC_BUTTON_RPAR,IDC_BUTTON_MSUB, IDC_BUTTON_PI, IDC_BUTTON_A, IDC_BUTTON_B, IDC_BUTTON_C, IDC_BUTTON_D, IDC_BUTTON_E, IDC_BUTTON_F,
    IDC_BUTTON_AVE, IDC_BUTTON_DMS,IDC_BUTTON_EXP, IDC_BUTTON_LN,   IDC_BUTTON_SIN,  IDC_BUTTON_XY,   IDC_BUTTON_LOG, IDC_BUTTON_SQR, IDC_BUTTON_CUBE,IDC_BUTTON_FACT,
    IDC_BUTTON_SUM, IDC_BUTTON_SIN,IDC_BUTTON_COS, IDC_BUTTON_TAN,  IDC_BUTTON_ASIN, IDC_BUTTON_ACOS, IDC_BUTTON_ATAN
};


uint currentAllocationSize = INITIAL_MEMORY_SIZE; // Current allocated memory size


/*
 * adjustMemoryAllocation
 *
 * Purpose:
 *    Sets the `currentAllocationSize` global variable based on the current calculator mode
 *    and precision level.
 *
 *
 * Remarks:
 *    - If the calculator is in scientific mode and the current precision level is less than
 *      `MAX_STANDARD_PRECISION`, the `currentAllocationSize` is set to
 *      `EXTENDED_MEMORY_SIZE`. This likely indicates that the calculator needs more
 *      memory to handle the higher precision calculations required in scientific mode.
 *    - Otherwise, `currentAllocationSize` is set to `INITIAL_MEMORY_SIZE`, which is
 *      presumably a smaller amount of memory suitable for standard mode calculations.
 */
void adjustMemoryAllocation(void) {
    if (isScientificMode() && calcState.currentPrecisionLevel < MAX_STANDARD_PRECISION) {
        currentAllocationSize = EXTENDED_MEMORY_SIZE;
    }
    else {
        currentAllocationSize = INITIAL_MEMORY_SIZE;
    }
}

/*
 * initCalcState()
 *
 * This function initializes the calculator's state, setting up all initial values
 * for the calculator parameters. It should be called once at the start of the application.
 *
 * The function performs the following tasks:
 * 1. Initializes string constants (class name, registry key, mode text)
 * 2. Sets the default help file path
 * 3. Initializes numeric values (precision, error codes, button states, etc.)
 * 4. Sets up the initial calculator mode
 * 5. Clears memory registers and error states
 * 6. Sets the default decimal separator
 * 7. Updates the decimal separator based on system settings
 *
 * @param None
 * @return None
 */
LRESULT CALLBACK calcWindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    int iVar;
    BOOL bVar;
    HMENU menuHandle;
    DWORD DVar;
    UINT commandID;
    POINT ptMouse;
    UINT menuItemID;
    WORD mouseX, mouseY;
    static BOOL isButtonPressed = FALSE;
    static DWORD currentPressedButtonID = INVALID_BUTTON;

    switch (uMsg)
    {
    case WM_ACTIVATE:
        iVar = (wParam == WA_ACTIVE) ? SW_SHOW : SW_HIDE;
        if (calcState.isScientificModeActive && calcState.scientificWindowHandle != NULL)
        {
            ShowWindow(calcState.scientificWindowHandle, iVar);
        }
        break;

    case WM_DESTROY:
        WinHelpA(calcState.windowHandle, calcState.helpFilePath, HELP_QUIT, 0);
        PostQuitMessage(0);
        return 0;

    case WM_SYSCOLORCHANGE:
        if (lParam == 0 ||
            lstrcmpA((LPCSTR)lParam, "colors") == 0 ||
            lstrcmpA((LPCSTR)lParam, "scheme") == 0)
        {
            initColors(0);
        }
        break;

    case WM_PAINT:
        refreshInterface();
        if (errorState == 0)
        {
            if (currentKeyPressed < KEY_RANGE_START || currentKeyPressed > KEY_RANGE_END)
            {
                updateDisplay();
            }
            else
            {
                DWORD tempHighPart = currentValueHighPart;
                DWORD tempAccumulated = accumulatedValue;
                currentValueHighPart = defaultPrecisionValue;
                accumulatedValue = lastValue;
                updateDisplay();
                accumulatedValue = tempAccumulated;
                currentValueHighPart = tempHighPart;
            }
        }
        else
        {
            handleCalculationError(errorCodeBase);
        }
        break;

    case WM_CLOSE:
        DestroyWindow(calcState.windowHandle);
        break;

    case WM_HELP:
        commandID = 0;
        if (windowHandle == (HWND)wParam)
        {
            ptMouse.x = GET_X_LPARAM(lParam);
            ptMouse.y = GET_Y_LPARAM(lParam);
            ScreenToClient(windowHandle, &ptMouse);
            commandID = getCalculatorButton((WORD)ptMouse.x, (WORD)ptMouse.y);
        }
        if (commandID == 0)
        {
            WinHelpA((HWND)wParam, calcState.helpFilePath, HELP_WM_HELP, HELP_CONTEXT_DATA);
            return 0;
        }
        bVar = handleContextHelp(calcState.windowHandle, calcState.appInstance, lParam);
        if (bVar)
        {
            if (commandID > MEMORY_BUTTON_START && commandID < MEMORY_BUTTON_END)
            {
                commandID = MEMORY_BUTTON_DEFAULT;
            }
            if (commandID > DIGIT_BUTTON_START && commandID < DIGIT_BUTTON_END)
            {
                commandID = DIGIT_BUTTON_DEFAULT;
            }
            WinHelpA((HWND)wParam, calcState.helpFilePath, HELP_CONTEXTMENU, commandID);
            return 0;
        }
        break;

    case WM_COMMAND:
        commandID = LOWORD(wParam);
        if (HIWORD(wParam) == 1 && commandID < MAX_COMMAND_ID)
        {
            for (int i = 0; i < 0x3d; i++)
            {
                if ((elementStateTable[i] >> 8 & 0xff) == commandID &&
                    (elementStateTable[i] & 3) != calcState.mode)
                {
                    updateButtonState(commandID, 100);
                    break;
                }
            }
        }
        if (commandID < 0x3d)
        {
            processButtonClick(commandID);
        }
        break;

    case WM_INITMENUPOPUP:
        if (IsClipboardFormatAvailable(CF_TEXT))
        {
            commandID = MF_ENABLED;
        }
        else
        {
            commandID = MF_GRAYED;
        }
        menuItemID = ID_EDIT_PASTE;
        menuHandle = GetMenu(windowHandle);
        EnableMenuItem(menuHandle, menuItemID, commandID);
        break;

    case WM_CTLCOLORSTATIC:
        iVar = GetDlgCtrlID((HWND)lParam);
        if (iVar == 0x19d || iVar == 0x19e)
        {
            HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);
            DVar = GetSysColor(COLOR_WINDOW);
            SetBkColor((HDC)wParam, DVar);
            DVar = GetSysColor(COLOR_WINDOWTEXT);
            SetTextColor((HDC)wParam, DVar);
            return (LRESULT)hBrush;
        }
        break;

    case WM_MOUSEMOVE:
        mouseX = LOWORD(lParam);
        mouseY = HIWORD(lParam);
        if (currentPressedButtonID != INVALID_BUTTON)
        {
            commandID = getCalculatorButton(mouseX, mouseY);
            if (commandID == currentPressedButtonID || isButtonPressed)
            {
                if (commandID == currentPressedButtonID && isButtonPressed)
                {
                    updateButtonState(currentPressedButtonID, STATE_DOWN);
                    isButtonPressed = FALSE;
                }
            }
            else
            {
                updateButtonState(currentPressedButtonID, STATE_UP);
                isButtonPressed = TRUE;
            }
        }
        break;

    case WM_LBUTTONDOWN:
        mouseX = LOWORD(lParam);
        mouseY = HIWORD(lParam);
        commandID = getCalculatorButton(mouseX, mouseY);
        if (commandID != 0)
        {
            currentPressedButtonID = commandID;
            updateButtonState(commandID, STATE_DOWN);
            isButtonPressed = FALSE;
            SetCapture(calcState.windowHandle);
        }
        break;

    case WM_LBUTTONUP:
        ReleaseCapture();
        mouseX = LOWORD(lParam);
        mouseY = HIWORD(lParam);
        commandID = getCalculatorButton(mouseX, mouseY);
        if (commandID == currentPressedButtonID)
        {
            if (commandID != 0)
            {
                updateButtonState(commandID, STATE_UP);
                isButtonPressed = TRUE;
                processButtonClick(commandID);
            }
        }
        currentPressedButtonID = INVALID_BUTTON;
        break;

    default:
        return DefWindowProcA(windowHandle, uMsg, wParam, lParam);
    }

    return result;
}

/*
 * configureCodePageSettings()
 *
 * Purpose:
 *     Configures character type flags based on the requested code page.
 *     This function updates the `charTypeFlags` array based on the character
 *     ranges defined for the given code page.  It handles both explicitly
 *     supported code pages and dynamically loaded code pages using the
 *     `GetCPInfo` function.
 *
 * Parameters:
 *     requestedCodepage:  The code page to configure. Special values:
 *                         - SYSTEM_CODE_PAGE: Use the system's default code page.
 *                         - Other values:   Attempt to set the specified code page.
 *
 * Return value:
 *     DWORD: Returns 0 on success or an error code on failure.
 *            ERROR_INVALID_PARAMETER is returned if the code page is not
 *            supported and is not a custom code page.
 *
 * Remarks:
 *     - This function is crucial for proper character handling, as it determines which
 *       characters are considered valid for numeric input, hexadecimal input, etc.
 *     - It makes use of the `charRangeTable` to define ranges of characters with
 *       specific properties.
 *     - For code pages not explicitly supported, it attempts to dynamically load
 *       code page information using `GetCPInfo`.
 *     - The function also handles DBCS (Double Byte Character Sets), marking
 *       lead bytes appropriately.
 *     - If the requested code page is invalid or unsupported, it resets the character
 *       type flags to their default state.
 */
DWORD configureCodePageSettings(int requestedCodepage)
{
    DWORD activeCodepage = setupCodePage(requestedCodepage);
    if (calcState.codepageInfo.currentCodepage == activeCodepage) {
        return 0;  // No change needed
    }

    if (activeCodepage != 0) {
        // Check if the active code page is in the supported list
        for (int i = 0; i < NUM_SUPPORTED_CODEPAGES; i++) {
            if (supportedCodepages[i] == activeCodepage) {
                // Initialize character type flags
                memset(charTypeFlags, 0, sizeof(charTypeFlags));

                // Set up character ranges for the code page
                for (int j = 0; j < 6; j++) { // Loop through all 6 ranges
                    CharRange* range = &charRangeTable[i * 6 + j];
                    if (range->start == 0 && range->end == 0) {
                        break; // No more ranges for this code page
                    }

                    for (DWORD charIndex = range->start; charIndex <= range->end; charIndex++) {
                        charTypeFlags[charIndex] |= range->flags;
                    }
                }

                // Set up code page specific settings
                calcState.codepageInfo.currentCodepage = activeCodepage;
                calcState.codepageInfo.codepageSpecificFlag = getPageSpecificFlag(activeCodepage);

                return 0;
            }
        }

        // If not in the supported list, try to get CP info
        CPINFO codepageInfo;
        if (GetCPInfo(activeCodepage, &codepageInfo)) {
            memset(charTypeFlags, 0, sizeof(charTypeFlags));

            if (codepageInfo.MaxCharSize >= 2) {
                // Set up lead byte ranges
                for (BYTE* leadBytePtr = codepageInfo.LeadByte; leadBytePtr[0] && leadBytePtr[1]; leadBytePtr += 2) {
                    for (DWORD i = leadBytePtr[0]; i <= leadBytePtr[1]; i++) {
                        charTypeFlags[i] |= CHAR_LEADBYTE;  // Mark as lead byte
                    }
                }

                // Mark single-byte characters
                for (uint i = 1; i < 0xFF; i++) {
                    charTypeFlags[i] |= 8;
                }

                calcState.codepageInfo.currentCodepage = activeCodepage;
                calcState.codepageInfo.codepageSpecificFlag = getPageSpecificFlag(activeCodepage);
            }
            else {
                calcState.codepageInfo.codepageSpecificFlag = 0;
                calcState.codepageInfo.currentCodepage = 0;
            }
            return 0;
        }

        if (!isCustomCodePage) {
            return ERROR_INVALID_PARAMETER;
        }
    }

    resetCharacterTypeFlags();
    return 0;
}


/*
 * initCalcState()
 *
 * This function initializes the calculator's state, setting up all initial values
 * for the calculator parameters. It should be called once at the start of the application.
 *
 * The function performs the following tasks:
 * 1. Initializes string constants (class name, registry key, mode text)
 * 2. Sets the default help file path
 * 3. Initializes numeric values (precision, error codes, button states, etc.)
 * 4. Sets up the initial calculator mode
 * 5. Clears memory registers and error states
 * 6. Sets the default decimal separator
 * 7. Updates the decimal separator based on system settings
 *
 * @param None
 * @return None
 */
void initCalcState(void)
{
    // Initialize string constants
    calcState.className = "CalculatorClass";
    calcState.registryKey = "SciCalc";
    calcState.modeText[STANDARD_MODE] = "Standard";
    calcState.modeText[SCIENTIFIC_MODE] = "Scientific";

    // Set default help file path
    strcpy_s(calcState.helpFilePath, MAX_PATH, "calc.hlp");

    // Initialize numeric values
    memset(calcState.accumulatedValue, 0, sizeof(calcState.accumulatedValue));
    calcState.appInstance = NULL;
    calcState.currentPrecisionLevel = MAX_STANDARD_PRECISION;
    calcState.codepageInfo.currentCodepage = GetACP(); //Gets system codepage
    calcState.currentValueHighPart = 0;
    calcState.defaultPrecisionValue = 0;
    calcState.decimalSeparator = DEFAULT_DECIMAL_SEPARATOR;
    calcState.errorCodeBase = 0;
    calcState.errorState = 0;
    calcState.hasOperatorPending = FALSE;
    calcState.keyPressed = INVALID_BUTTON;
    calcState.lastValue = 0;
    calcState.mode = STANDARD_MODE;
    calcState.memoryRegister[0] = 0;
    calcState.memoryRegister[1] = 0;
    calcState.numberBase = 10;
    calcState.scientificNumber.exponent = 0;
    calcState.scientificNumber.mantissaLow = 0;
    calcState.scientificNumber.mantissaHigh = 0;
    calcState.statisticsWindowOpen = FALSE;
    calcState.windowHandle = NULL;

    updateDecimalSeparator();

    // Additional initialization steps
    //InitializeRandomSeed(GetTickCount());  // Seed the random number generator
    //setNumberFormatting();  // Set up number formatting functions
    //configureFPUPrecision();  // Configure floating-point unit precision
}

void initApplicationCodePage(void)
{
    configureCodePageSettings(SYSTEM_CODE_PAGE);
}

/*
 * initApplicationPath
 *
 * Purpose:
 *     Initializes the application path, storing the full path and its individual
 *     components in the `calcState.appPath` structure. This function is called
 *     during the initialization of the calculator application.
 *
 *
 * Remarks:
 *     1. Obtains the full path of the calculator executable using `GetModuleFileNameA`.
 *     2. Calls `tokenizeString` twice:
 *         - The first call is to determine the number of path components and the required memory.
 *         - The second call performs the actual tokenization, splitting the full path into
 *           individual components (separated by backslashes or forward slashes).
 *     3. Allocates memory for the path component pointers and the string data.
 *     4. Stores the full path, the array of path component pointers, and the component count
 *        in the `calcState.appPath` structure.
 */
void initApplicationPath(void)
{
    char** pathComponents;
    char* appPathBuffer;
    int pathDataSize;
    int componentCount;

    // Get the full path of the current executable (max 260 characters)
    GetModuleFileNameA((HMODULE)0x0, calcState.appPath.fullPath, 0x104);
    appPathBuffer = calcState.appPath.fullPath;

    // First pass: count path components and calculate required memory
    tokenizeString(appPathBuffer, (char**)0x0, (char*)0x0, &componentCount, &pathDataSize);

    // Allocate memory for path components and string data
    pathComponents = (char**)allocateMemoryFromHeap(componentCount * 4 + pathDataSize);
    if (pathComponents == (char**)0x0) {
        showRunTimeError(8);  // Memory allocation error
        return;
    }

    // Second pass: actually tokenize the path
    tokenizeString(appPathBuffer, pathComponents, (char*)(pathComponents + componentCount),
        &componentCount, &pathDataSize);

    // Store results in calcState structure
    calcState.appPath.components = pathComponents;        // Store the path components 
    calcState.appPath.componentCount = componentCount - 1; // Store the component count 
}

/*
 * initCalcRuntime()
 *
 * Purpose:
 *     Initializes the calculator runtime environment by setting up number
 *     formatting and configuring the floating-point unit (FPU) precision.
 *
 * Parameters:
 *     initializationFlags:  Flags to control initialization.  (Not used in this
 *                          implementation).
 *
 * Return Value:
 *     DWORD:  Always returns 0, indicating successful initialization.
 */
DWORD initCalcRuntime(int initializationFlags)
{
    //Functions below need to be implemetned
    // Set up number formatting 
    setNumberFormatting();

    // Configure FPU precision
    configureFPUPrecision();

    return 0; // Success 
}

/*
 * initColors()
 *
 * This function initializes and updates the color scheme and layout of the calculator application.
 * It handles both standard and scientific modes, adjusting the interface accordingly.
 *
 * The function performs the following tasks:
 * 1. Determines the background color based on the current calculator mode
 * 2. Checks if the background color or decimal separator has changed
 * 3. Updates the calculator's layout and dimensions based on the current mode
 * 4. Adjusts the main window size and position
 * 5. Updates the menu to reflect the current mode
 * 6. Redraws the window background with the new color
 * 7. Shows or hides interface elements based on the current mode
 * 8. Sets up the scientific mode if necessary
 * 9. Updates the mode text display
 *
 * This function is crucial for maintaining the visual consistency and proper layout
 * of the calculator across different modes and system settings.
 *
 * @param forceUpdate     If true, forces a complete update of the interface regardless of changes
 */
void initColors(int forceUpdate)
{
    char previousDecimalSeparator;
    DWORD backgroundColor;
    BOOL isIconic;
    int windowHeight, windowWidth;
    HMENU menuHandle;
    HDC hDC;
    HBRUSH hBackgroundBrush;
    HWND hChildWindow;
    LPCSTR* mode_text_ptr;
    WORD* pWindowState;
    BOOL backgroundColorChanged;
    char backgroundColorString[20];
    RECT windowRect;
    int standardModeWidth, standardModeHeight;
    int scientificModeWidth, scientificModeHeight;
    static int cxChar, cyChar;
    const char* currentModeText = calcState.modeText[calcState.mode];
    int modeTextID;
    int totalWidth; //Total width of the calculator

    int VERTICAL_OFFSET = 0;
    int BUTTON_BASE_SIZE = 0;

    // Determine background color based on calculator display mode
    if (calcState.mode == SCIENTIFIC_MODE) {
        GetProfileStringA(calcState.registryKey, "background", "8421504", backgroundColorString, sizeof(backgroundColorString));
        calcState.isHighContrastMode = FALSE;
    }
    else {
        GetProfileStringA(calcState.registryKey, "background", DEFAULT_BACKGROUND_COLOR, backgroundColorString, sizeof(backgroundColorString));
    }

    // Convert background color string to DWORD value
    backgroundColor = (backgroundColorString[0] == '-') ? GetSysColor(COLOR_BTNFACE) : parseSignedInteger(backgroundColorString);

    // Check if background color has changed
    previousDecimalSeparator = calcState.decimalSeparator;
    backgroundColorChanged = (backgroundColor != calcState.currentBackgroundColor);
    if (backgroundColorChanged) {
        calcState.currentBackgroundColor = backgroundColor;
    }

    // Get decimal separator from system settings
    GetProfileStringA(CALCULATOR_APP_NAME, "sDecimal", DEFAULT_DECIMAL_SEPARATOR, &calcState.decimalSeparator, sizeof(calcState.decimalSeparator));

    // Update interface if necessary
    if ((previousDecimalSeparator != calcState.decimalSeparator) || backgroundColorChanged || forceUpdate) {
        SetDecimalSeparator(&calcState);

        TEXTMETRIC tm;
        GetTextMetrics(hDC, &tm);
        cxChar = tm.tmAveCharWidth;
        cyChar = tm.tmHeight + tm.tmExternalLeading;

        isIconic = IsIconic(calcWindows.main);
        if (!isIconic) {
            if (IsWindowVisible(calcWindows.main)) {
                SetFocus(calcWindows.main);
            }
            InvalidateRect(calcWindows.main, NULL, TRUE);

            // Set dimensions based on calculator mode
            if (calcState.mode == STANDARD_MODE) {
                windowRect.left = 0;
                windowRect.top = 0;
                windowRect.bottom = 0;
                windowRect.right = 7;
                MapDialogRect(calcWindows.main, &windowRect);
                VERTICAL_OFFSET = windowRect.right - 6;

                windowRect.right = 24;
                windowRect.bottom = 18;
                windowRect.top = 0;
                MapDialogRect(calcWindows.main, &windowRect);
                BUTTON_BASE_SIZE = windowRect.right;

                windowRect.right = 8;
                windowRect.left = 0;
                MapDialogRect(calcWindows.main, &windowRect);

                int horizontalDialogUnits = (STANDARD_CALC_COLS * BUTTON_BASE_SIZE) +
                    ((STANDARD_CALC_COLS - 1) * BUTTON_HORIZONTAL_SPACING) +
                    (2 * HORIZONTAL_MARGIN); // Adjust calculation based on layout
                int verticalDialogUnits = (STANDARD_CALC_ROWS * BUTTON_BASE_SIZE) +
                    ((STANDARD_CALC_ROWS - 1) * BUTTON_VERTICAL_SPACING) +
                    (2 * VERTICAL_MARGIN);   // Adjust calculation based on layout

                // Explicitly cast to double for floating-point division
                standardModeWidth = (int)(((double)horizontalDialogUnits * cxChar) / 4.0);
                standardModeHeight = (int)(((double)verticalDialogUnits * cyChar) / 8.0);

                windowWidth = standardModeWidth;
                windowHeight = standardModeHeight;
            }
            else {  // Scientific mode

                int horizontalDialogUnits = (SCIENTIFIC_CALC_COLS * BUTTON_BASE_SIZE) +
                    ((SCIENTIFIC_CALC_COLS - 1) * calcState.buttonHorizontalSpacing) +
                    (2 * HORIZONTAL_MARGIN); // Adjust for scientific mode layout
                totalWidth = (int)(((double)horizontalDialogUnits * cxChar) / 4.0);

                scientificModeWidth = (totalWidth * 180) / 326;

                windowRect.left = 0;
                windowRect.top = 0;
                windowRect.bottom = 0;
                windowRect.right = 7;
                MapDialogRect(calcWindows.main, &windowRect);
                VERTICAL_OFFSET = windowRect.right - 6;

                windowRect.right = 24;
                windowRect.bottom = 18;
                windowRect.top = 0;
                MapDialogRect(calcWindows.main, &windowRect);
                BUTTON_BASE_SIZE = windowRect.right;

                windowRect.right = 8;
                windowRect.left = 0;
                MapDialogRect(calcWindows.main, &windowRect);
                windowHeight = (standardModeHeight * 4) / 5;
                windowWidth = scientificModeWidth;
            }

            // Update main window position and size
            calcState.buttonHorizontalSpacing = windowRect.right;
            SetWindowPos(calcWindows.main, NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);

            // Update menu to reflect current mode
            menuHandle = GetMenu(calcWindows.main);
            menuHandle = GetSubMenu(menuHandle, 1);
            CheckMenuItem(menuHandle, calcState.mode, MF_BYCOMMAND | MF_CHECKED);
            CheckMenuItem(menuHandle, 1 - calcState.mode, MF_BYCOMMAND | MF_UNCHECKED);

            // Fill window background with new color
            SetRect(&windowRect, 0, 0, standardModeWidth, standardModeHeight);
            hDC = GetDC(calcWindows.main);
            hBackgroundBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            FillRect(hDC, &windowRect, hBackgroundBrush);
            ReleaseDC(calcWindows.main, hDC);

            // Update visibility of interface elements
            pWindowState = &windowStateTable[0];
            do {
                hChildWindow = GetDlgItem(calcWindows.main, *pWindowState & 0x7fff);
                if (hChildWindow != NULL) {
                    ShowWindow(hChildWindow, (*pWindowState >> 15) == calcState.mode);
                }
                pWindowState++;
            } while (pWindowState < &windowStateTable[15]);

            // Set up scientific mode if necessary
            if (calcState.mode != SCIENTIFIC_MODE) {
                pass;
                //setupCalculatorMode(123);
            }

            if ((calcState.memoryRegister[1] & 0x7fffffff | calcState.memoryRegister[0]) == 0) {
                currentModeText = calcState.modeText[calcState.mode];
            }

            if (calcState.mode == STANDARD_MODE) {
                modeTextID = IDC_TEXT_STANDARD_MODE;
            }
            else {
                modeTextID = IDC_TEXT_SCIENTIFIC_MODE;
            }

            SetDlgItemTextA(calcWindows.main, modeTextID, currentModeText);
        }
    }
}

/*
 * initializeEnvironmentVariables()
 *
 * This function initializes the environment variables for the calculator application,
 * making them accessible for later use.  It retrieves the system's environment
 * block, parses it to extract individual environment variables, and stores them
 * in the `envVariables` structure.
 *
 * The function performs the following tasks:
 * 1. Gets the environment block using GetEnvironmentStringsA().
 * 2. Iterates through the block, counting the number of environment variables.
 * 3. Allocates a single contiguous block of memory to store both:
 *     - An array of pointers to the start of each environment variable string.
 *     - The environment strings themselves (name=value pairs).
 * 4. Populates the pointer array and copies the environment strings to the data area.
 * 5. Null-terminates the pointer array to indicate the end of the list.
 * 6. Assigns the initialized `envVariables` structure to `calcState.environmentVariables`,
 *    making the environment variables accessible through the calculator's state.
 *
 * This function is called during calculator initialization to set up access to
 * environment variables that the calculator might need to use.
 * 
 */
void initEnvironmentVariables(void)
{
    char* envString = GetEnvironmentStringsA(); // Get environment strings
    int envVarCount = 0;

    // Count the number of environment variables
    while (*envString != '\0') {
        if (*envString != '=') {
            envVarCount++;
        }
        // Move to next environment string 
        while (*envString++ != '\0');
    }

    // Allocate memory for environment variable pointers and string data
    size_t totalSize = (envVarCount + 1) * sizeof(int) + strlen(envString) + 1;
    char** envVarArray = (char**)allocateMemoryFromHeap(totalSize);
    if (envVarArray == NULL) {
        showRunTimeError(9);  // Memory allocation error
        return;
    }

    // Initialize envVariables (instead of directly accessing calcState)
    envVariables.variables = envVarArray;
    envVariables.count = envVarCount;

    // Copy environment strings to the allocated memory
    char* destPtr = (char*)(envVarArray + envVarCount + 1); // Data area starts after pointers
    envString = GetEnvironmentStringsA(); // Reset envString to the beginning 
    while (*envString != '\0') {
        if (*envString != '=') {
            *envVarArray++ = destPtr; // Store pointer to variable name in the pointer array
            strcpy(destPtr, envString); // Copy the entire environment string (name=value)
            destPtr += strlen(destPtr) + 1;  // Move destPtr to the end of the copied string 
        }
        // Move to the next environment string
        while (*envString++ != '\0');
    }

    // Null-terminate the environment variable array 
    *envVarArray = NULL;

}

/*
 * initStandardStreams()
 *
 * Purpose:
 *     Initializes the standard input, output, and error streams for the
 *     calculator application. This function is called early in the application's
 *     startup process to set up how the calculator interacts with the console
 *     or any redirected input and output.
 *
 * Remarks:
 *     This function retrieves information about the startup environment,
 *     including any redirection of standard streams (input, output, and error).
 *     It handles cases where the streams are redirected to files or pipes and
 *     initializes the standard stream handles accordingly.
 *     It determines the type of each stream (character device, pipe, or file)
 *     and sets corresponding flags in `calcState.standardStreamFlags`.
 */
void initStandardStreams(void)
{
    STARTUPINFOA startupInfo;
    DWORD streamType;
    HANDLE fileHandle;

    // Get startup information, which includes details about redirected streams.
    GetStartupInfoA(&startupInfo);

    // Initialize the standard input stream (stdin)
    streams.standardStreamHandles[0] = GetStdHandle(STD_INPUT_HANDLE);
    if (streams.standardStreamHandles[0] != INVALID_HANDLE_VALUE) {
        streams.standardStreamFlags[0] = STREAM_VALID; // Stream is valid and open.
        streamType = GetFileType(streams.standardStreamHandles[0]);
        if ((streamType & FILE_TYPE_PIPE) == FILE_TYPE_CHAR) {
            streams.standardStreamFlags[0] |= STREAM_CONSOLE; // Stream is a character device (console).
        }
        else if ((streamType & FILE_TYPE_PIPE) == FILE_TYPE_PIPE) {
            streams.standardStreamFlags[0] |= STREAM_PIPE; // Stream is a pipe.
        }
    }

    // Initialize the standard output stream (stdout)
    streams.standardStreamHandles[1] = GetStdHandle(STD_OUTPUT_HANDLE);
    if (streams.standardStreamHandles[1] != INVALID_HANDLE_VALUE) {
        streams.standardStreamFlags[1] = STREAM_VALID; // Stream is valid and open.
        streamType = GetFileType(streams.standardStreamHandles[1]);
        if ((streamType & FILE_TYPE_PIPE) == FILE_TYPE_CHAR) {
            streams.standardStreamFlags[1] |= STREAM_CONSOLE; // Stream is a character device (console).
        }
        else if ((streamType & FILE_TYPE_PIPE) == FILE_TYPE_PIPE) {
            streams.standardStreamFlags[1] |= STREAM_PIPE; // Stream is a pipe.
        }
    }

    // Initialize the standard error stream (stderr)
    streams.standardStreamHandles[2] = GetStdHandle(STD_ERROR_HANDLE);
    if (streams.standardStreamHandles[2] != INVALID_HANDLE_VALUE) {
        streams.standardStreamFlags[2] = STREAM_VALID; // Stream is valid and open.
        streamType = GetFileType(streams.standardStreamHandles[2]);
        if ((streamType & FILE_TYPE_PIPE) == FILE_TYPE_CHAR) {
            streams.standardStreamFlags[2] |= STREAM_CONSOLE; // Stream is a character device (console).
        }
        else if ((streamType & FILE_TYPE_PIPE) == FILE_TYPE_PIPE) {
            streams.standardStreamFlags[2] |= STREAM_PIPE; // Stream is a pipe.
        }
    }
}
/*
 * initInstance()
 *
 * This function creates and initializes the main window for the calculator application.
 * It sets up the window, configures its properties, and prepares it for display.
 *
 * The function performs the following tasks:
 * 1. Creates the main window using CreateWindowExA with specified styles and dimensions
 * 2. Checks if window creation was successful
 * 3. Sets up a RECT structure for proper button measurement
 * 4. Uses MapDialogRect to convert dialog units to pixels
 * 5. Initializes the BUTTON_BASE_SIZE for consistent UI scaling
 * 6. Shows and updates the main window
 *
 * @param appInstance     Handle to the current instance of the application
 * @param windowMode      Controls how the window is to be shown (e.g., maximized, minimized)
 * @return                true if window creation and initialization succeed, false otherwise
 */
BOOL initInstance(HINSTANCE appInstance, int windowMode)
{
    calcState.windowHandle = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        calcState.className,
        "Calculator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 320,
        NULL, NULL, appInstance, NULL);

    //For proper button measurement and initialization.
    RECT windowRect;

    if (calcState.windowHandle == NULL)
    {
        return FALSE;
    }

    // Set up initial rectangle
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = 24;
    windowRect.bottom = 18;

    // Map dialog units to pixels
    MapDialogRect(calcState.windowHandle, &windowRect);

    // Set the button base size
    BUTTON_BASE_SIZE = windowRect.right;

    ShowWindow(calcState.windowHandle, windowMode);
    UpdateWindow(calcState.windowHandle);

    return TRUE;
}

/*
 * getCalculatorButton()
 *
 * Purpose:
 *     Determines the ID of the calculator button that was clicked based on the
 *     mouse coordinates. It handles both standard and scientific modes and
 *     considers the layout and dimensions of the buttons.
 *
 * Parameters:
 *     mouseX: The x-coordinate of the mouse click in client coordinates.
 *     mouseY: The y-coordinate of the mouse click in client coordinates.
 *
 * Return Value:
 *     DWORD: The ID of the button that was clicked, or 0 if no button was clicked.
 */
DWORD getCalculatorButton(ushort mouseX, ushort mouseY)
{
    int buttonWidth, horizontalPosition, rowIndex;
    RECT clientRect;

    // Calculate the width of a standard button
    buttonWidth = (BUTTON_BASE_SIZE * SPECIAL_BUTTON_WIDTH_FACTOR) / 3 + 5;
    horizontalPosition = 0;
    rowIndex = (BUTTON_BASE_SIZE * SPECIAL_BUTTON_WIDTH_FACTOR) / 3 + 1;

    // Define the main button area based on the current mode
    int topEdge, bottomEdge, leftEdge, rightEdge;

    if (calcState.mode == SCIENTIFIC_MODE) {
        topEdge = VERTICAL_MARGIN;
        bottomEdge = topEdge + SCIENTIFIC_CALC_ROWS * (BUTTON_BASE_SIZE + BUTTON_VERTICAL_SPACING) - BUTTON_VERTICAL_SPACING;
        leftEdge = HORIZONTAL_MARGIN;
        rightEdge = leftEdge + SCIENTIFIC_CALC_COLS * (BUTTON_BASE_SIZE + SCIENTIFIC_BUTTON_EXTRA_WIDTH + BUTTON_HORIZONTAL_SPACING) - BUTTON_HORIZONTAL_SPACING;
    }
    else { // Standard mode
        topEdge = VERTICAL_MARGIN + (BUTTON_BASE_SIZE * SPECIAL_BUTTON_HEIGHT_FACTOR) / 2 + BUTTON_VERTICAL_SPACING;
        bottomEdge = topEdge + STANDARD_CALC_ROWS * (BUTTON_BASE_SIZE + BUTTON_VERTICAL_SPACING) - BUTTON_VERTICAL_SPACING;
        leftEdge = HORIZONTAL_MARGIN;
        rightEdge = leftEdge + STANDARD_CALC_COLS * (BUTTON_BASE_SIZE + BUTTON_HORIZONTAL_SPACING) - BUTTON_HORIZONTAL_SPACING;
    }

    if (mouseY >= topEdge && mouseY < bottomEdge) {
        int verticalPosition = VERTICAL_OFFSET + 6; // Check if VERTICAL_OFFSET is defined correctly

        if (mouseX >= leftEdge && mouseX <= rightEdge) {
            // Find the column of the clicked button
            int column = 0;
            BOOL buttonFound = FALSE;
            int buttonsPerRow = (calcState.mode == SCIENTIFIC_MODE) ? SCIENTIFIC_CALC_COLS : STANDARD_CALC_COLS;

            // Calculate the height of a button row 
            int buttonRowHeight = (BUTTON_ROW_HEIGHT_FACTOR * BUTTON_BASE_SIZE + 7) >> 3;

            while (column < buttonsPerRow && !buttonFound) {
                if (mouseY >= topEdge + column * buttonRowHeight &&
                    mouseY <= topEdge + column * buttonRowHeight + ((MAIN_BUTTON_HEIGHT_FACTOR * BUTTON_BASE_SIZE + 7) >> 3)) {
                    buttonFound = TRUE;
                }
                column++;
            }

            if (buttonFound && column <= buttonsPerRow) {
                // Find the row of the clicked button
                int row = 0;
                BOOL rowFound = FALSE;

                while (row < (calcState.mode == SCIENTIFIC_MODE ? SCIENTIFIC_CALC_ROWS : STANDARD_CALC_ROWS) && !rowFound) {
                    horizontalPosition = adjustButtonHorizontalPosition(horizontalPosition, row, 0);
                    if (mouseX >= horizontalPosition + verticalPosition &&
                        mouseX <= horizontalPosition + verticalPosition + BUTTON_BASE_SIZE) {
                        rowFound = TRUE;
                    }
                    horizontalPosition += BUTTON_BASE_SIZE + 4;
                    row++;
                }

                if (rowFound) {
                    // Calculate button index and return button ID
                    int buttonIndex = buttonsPerRow * (row - 1) + column - 1;

                    // Use the correct BUTTON_ID_MAP based on the mode
                    if (calcState.mode == SCIENTIFIC_MODE) {
                        if (buttonIndex < sizeof(BUTTON_ID_MAP_SCIENTIFIC) / sizeof(DWORD)) {
                            return BUTTON_ID_MAP_SCIENTIFIC[buttonIndex];
                        }
                    }
                    else { // Standard mode
                        if (buttonIndex < sizeof(BUTTON_ID_MAP_STANDARD) / sizeof(DWORD)) {
                            return BUTTON_ID_MAP_STANDARD[buttonIndex];
                        }
                    }
                }
            }
        }
    }
    else {
        // Check for special buttons at the top
        GetClientRect(calcState.windowHandle, &clientRect);

        for (int i = 0; i < 3; i++) {
            if (mouseX <= clientRect.right - horizontalPosition - (calcState.mode == 0 ? 1 : 0) - 10 &&
                mouseX > clientRect.right - horizontalPosition - rowIndex - (calcState.mode == 0 ? 1 : 0) - 10) {
                return i + SPECIAL_BUTTON_OFFSET;
            }
            horizontalPosition += buttonWidth;
        }
    }

    return 0;  // No button found
}

/*
 * getStatusCode()
 *
 * This function retrieves a status message corresponding to a given status code.
 * It uses the STATUS_MESSAGE_TABLE to directly access the message.
 *
 * The function performs the following tasks:
 * 1. Checks if the given statusCode is within the valid range
 * 2. If valid, returns the corresponding message from STATUS_MESSAGE_TABLE
 * 3. If invalid, returns NULL
 *
 * @param statusCode The status code to look up
 * @return The corresponding status message, or NULL if the status code is invalid
 */
const char* getStatusCode(int statusCode)
{
    if ((statusCode >= 0) && (statusCode < STATUS_MESSAGE_TABLE_END)) {
        return STATUS_MESSAGE_TABLE[statusCode];
    }
    else {
        return NULL; // No matching status message found
    }
}

void handleCalculationError(int errorCode)
{
    LPCSTR errorMessage = getStatusCode(errorCode);
    if (errorMessage != NULL) {
        MessageBoxA(NULL, errorMessage, "Runtime Error", MB_ICONERROR);
    }
}

/*
 * refreshInterface()
 *
 * This function is responsible for redrawing the entire calculator interface.
 * It handles both standard and scientific modes, adjusting the layout accordingly.
 *
 * The function performs the following tasks:
 * 1. Determines the current calculator mode and sets appropriate dimensions
 * 2. Sets up colors and cursor for drawing
 * 3. Draws the calculator frame
 * 4. Calculates button dimensions based on the current mode
 * 5. Iterates through all buttons, drawing each visible button and its text
 * 6. Handles high contrast mode for accessibility
 *
 * This function is typically called when the calculator needs a full redraw,
 * such as after mode switches, resizing, or when the window is uncovered.
 *
 * No parameters.
 * No return value.
 */
void refreshInterface(void)
{
    PAINTSTRUCT ps;
    RECT clientRect, buttonRect, edgeRect;
    SIZE textSize;
    HGDIOBJ oldFont, oldBrush;
    HCURSOR oldCursor;
    COLORREF backgroundColor, textColor;
    HDC hdc;
    int row, col, buttonIndex;
    int buttonWidth, buttonHeight, buttonSpacing;
    int textLength, textX, textY;
    char* buttonText;
    int calcRows, calcCols;
    BOOL isHighContrastMode;

    // Check for high contrast mode using SystemParametersInfo
    SystemParametersInfo(SPI_GETHIGHCONTRAST, 0, &isHighContrastMode, 0);

    // Determine calculator dimensions based on mode
    if (calcState.mode == SCIENTIFIC_MODE) {
        calcRows = SCIENTIFIC_CALC_ROWS;
        calcCols = SCIENTIFIC_CALC_COLS;
    }
    else {
        calcRows = STANDARD_CALC_ROWS;
        calcCols = STANDARD_CALC_COLS;
    }

    // Set up colors and cursor
    backgroundColor = GetSysColor(COLOR_BTNFACE);
    textColor = GetSysColor(COLOR_BTNTEXT);
    oldCursor = SetCursor(LoadCursorA(NULL, IDC_ARROW));
    ShowCursor(TRUE);

    // Begin painting
    hdc = BeginPaint(calcState.windowHandle, &ps);
    oldFont = SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
    oldBrush = SelectObject(hdc, GetSysColorBrush(COLOR_BTNFACE));

    // Draw calculator frame
    GetClientRect(calcState.windowHandle, &clientRect);
    edgeRect = (RECT){ 1, 5, clientRect.right - 1, 8 };
    DrawEdge(hdc, &edgeRect, EDGE_SUNKEN, BF_RECT);

    // Set up button dimensions
    buttonWidth = BUTTON_BASE_SIZE + (calcState.mode == SCIENTIFIC_MODE ? SCIENTIFIC_BUTTON_EXTRA_WIDTH : 0);
    buttonHeight = BUTTON_BASE_SIZE;
    buttonSpacing = 4;

    // Draw buttons
    SetBkMode(hdc, TRANSPARENT);
    buttonIndex = 0;
    for (row = 0; row < calcRows; row++) {
        for (col = 0; col < calcCols; col++) {
            if (isButtonVisible(buttonIndex, calcState.mode)) {
                int x = col * (buttonWidth + buttonSpacing) + HORIZONTAL_MARGIN;
                int y = row * (buttonHeight + buttonSpacing) + VERTICAL_MARGIN;

                // Draw button
                buttonRect = (RECT){ x, y, x + buttonWidth, y + buttonHeight };
                DrawEdge(hdc, &buttonRect, EDGE_RAISED, BF_RECT);

                // Draw button text
                buttonText = getButtonText(buttonIndex);
                textLength = lstrlenA(buttonText);
                GetTextExtentPointA(hdc, buttonText, textLength, &textSize);
                textX = x + (buttonWidth - textSize.cx) / 2;
                textY = y + (buttonHeight - textSize.cy) / 2;

                if (isHighContrastMode) {
                    SetTextColor(hdc, getElementColor(buttonIndex, backgroundColor, textColor));
                }
                TextOutA(hdc, textX, textY, buttonText, textLength);
            }
            buttonIndex++;
        }
    }

    // Clean up
    SelectObject(hdc, oldFont);
    SelectObject(hdc, oldBrush);
    EndPaint(calcState.windowHandle, &ps);
    SetCursor(oldCursor);
    ShowCursor(FALSE);
}

BOOL handleContextHelp(HWND hwnd, HINSTANCE hInstance, UINT message)
{
    // ID do recurso do menu de contexto
    const UINT CONTEXT_MENU_RESOURCE_ID = 4;

    // Flags para o TrackPopupMenuEx
    const UINT TRACK_POPUP_FLAGS = TPM_RETURNCMD | TPM_RIGHTBUTTON;

    // Carrega o menu de contexto dos recursos da aplica��o
    HMENU menuHandle = LoadMenuA(hInstance, MAKEINTRESOURCE(CONTEXT_MENU_RESOURCE_ID));
    if (menuHandle == NULL) {
        // Falha ao carregar o menu
        return FALSE;
    }

    // Obt�m o primeiro (e provavelmente �nico) submenu
    HMENU hPopupMenu = GetSubMenu(menuHandle, 0);
    if (hPopupMenu == NULL) {
        // Falha ao obter o submenu
        DestroyMenu(menuHandle);
        return FALSE;
    }

    // Extrai as coordenadas x e y do mouse do par�metro message
    WORD xPos = LOWORD(message);
    WORD yPos = HIWORD(message);

    // Exibe o menu de contexto na posi��o do mouse
    UINT result = TrackPopupMenuEx(
        hPopupMenu,
        TRACK_POPUP_FLAGS,
        xPos,
        yPos,
        hwnd,
        NULL  // N�o usamos TPMPARAMS neste caso
    );

    // Limpa o menu da mem�ria
    DestroyMenu(menuHandle);

    // Verifica se o item "What's This?" foi selecionado (ID 8)
    return (result == 8);
}


BOOL hasDecimalSeparator(const char* str) {
    if (str == NULL) {
        return FALSE;
    }

    while (*str != '\0') {
        if (*str == calcState.decimalSeparator) {
            return TRUE;
        }
        str++;
    }

    return FALSE;
}

/*
 * processButtonClick
 *
 * This function handles the processing of button clicks in the calculator application.
 * It manages input mode, performs calculations, and updates the calculator state based on user input.
 *
 * The function performs the following tasks:
 * 1. Checks if the pressed key is a special function key
 * 2. Handles error states and input mode activation
 * 3. Processes numeric input and operator input
 * 4. Manages calculator state reset for certain key combinations
 * 5. Performs calculations and updates the calculator state
 * 6. Handles special cases (scientific mode, parentheses, etc.)
 * 7. Updates the display after processing
 *
 * This function is central to the calculator's operation, interpreting user input
 * and managing the flow of calculations. It works in conjunction with other functions
 * like updateInputMode, appendDigit, and performAdvancedCalculation to provide
 * a complete calculation experience.
 *
 * @param currentKeyPressed    The key code of the button that was pressed
 *
 * No return value.
 */
void processButtonClick(uint currentKeyPressed)
{
    BOOL isValidInput;
    double calculationResult;
    int currentOperatorPrecedence, newOperatorPrecedence, stackPointer;

    // Handle special function keys
    if (!isSpecialFunctionKey(currentKeyPressed)) {
        calcState.keyPressed = currentKeyPressed;
    }

    // Handle error state
    if (calcState.errorState != 0 && !isClearKey(currentKeyPressed)) {
        MessageBeep(0);
        return;
    }

    // Handle input mode activation
    if (!calcState.isInputModeActive) {
        isValidInput = isNumericInput(currentKeyPressed) || currentKeyPressed == IDC_BUTTON_DOT;
        if (isValidInput) {
            calcState.isInputModeActive = TRUE;
            initCalcState();
        }
    }
    else if (isOperatorKey(currentKeyPressed) || currentKeyPressed == IDC_BUTTON_EXP) {
        calcState.isInputModeActive = FALSE;
    }

    // Reset calculator state for certain key combinations
    if (isNumericInput(currentKeyPressed) &&
        (isPreviousKeyOperator() || calcState.keyPressed == IDC_BUTTON_RPAR || currentKeyPressed == IDC_BUTTON_EXP)) {
        resetCalculatorState();
    }

    // Process numeric input
    if (isNumericInput(currentKeyPressed)) {
        int digit = convertKeyToDigit(currentKeyPressed);
        if (digit < calcState.numberBase) {
            if (calcState.numberBase == 10) {
                if (!appendDigit(calcState.accumulatedValue, digit)) { // Pass the string
                    MessageBeep(0);
                    return;
                }
            }
            else {
                if (isValueOverflow(calcState.accumulatedValue, calcState.numberBase, digit)) {
                    handleCalculationError(STATUS_OVERFLOW);
                    return;
                }
                // Convert accumulatedValue to an integer, perform the calculation,
                // then convert back to a string.
                LONGLONG accumulatedInt = strtol(calcState.accumulatedValue, NULL, calcState.numberBase);
                accumulatedInt = calcState.numberBase * accumulatedInt + digit * calcState.currentSign;
                _ltoa(accumulatedInt, calcState.accumulatedValue, calcState.numberBase);
            }
        }
        else {
            MessageBeep(0);
        }
        updateDisplay();
        return;
    }

    // Handle statistical functions
    if (currentKeyPressed >= IDC_BUTTON_STAT_RED && currentKeyPressed <= IDC_BUTTON_STAT_CAD) {
        if (calcState.statisticsWindowOpen) {
            performStatisticalCalculation(currentKeyPressed);
            if (calcState.errorState == 0) {
                updateDisplay();
            }
        }
        else {
            MessageBeep(0);
        }
        calcState.isInverseMode = FALSE;
        updateToggleButton(IDC_BUTTON_INV, FALSE);
        return;
    }

    // Handle parentheses
    if (currentKeyPressed == IDC_BUTTON_LPAR) {
        pushOperator(IDC_BUTTON_LPAR, 0); // Push left parenthesis onto the stack
    }
    else if (currentKeyPressed == IDC_BUTTON_RPAR) {
        // Evaluate expressions until we find a left parenthesis
        while (calcState.operatorStackPointer > 0 && getTopOperator() != IDC_BUTTON_LPAR) {
            if (calcState.operatorStackPointer > 0) {
                uint operator = popOperator();
                double operand2 = popOperand();
                double operand1 = popOperand();
                double result = performAdvancedCalculation(operator, operand1, operand2);
                pushOperand(result);
            }
        }
        if (calcState.operatorStackPointer > 0 && getTopOperator() == IDC_BUTTON_LPAR) {
            popOperator(); // Remove the left parenthesis
        }
        else {
            MessageBeep(0); // Error: Unmatched closing parenthesis
            return;
        }
    }

    // Process operator input
    if (isOperatorKey(currentKeyPressed)) {
        if (calcState.hasOperatorPending) {
            do {
                stackPointer = calcState.operatorStackPointer;
                newOperatorPrecedence = getOperatorPrecedence(currentKeyPressed);
                currentOperatorPrecedence = getOperatorPrecedence(calcState.operator);

                if (newOperatorPrecedence > currentOperatorPrecedence && calcState.mode == STANDARD_MODE) {
                    if (calcState.operatorStackPointer < MAX_OPERATOR_STACK) {
                        pushOperator(calcState.currentOperator, calcState.lastValue);
                    }
                    else {
                        calcState.operatorStackPointer = MAX_OPERATOR_STACK - 1;
                        MessageBeep(0);
                    }
                    calcState.lastValue = calcState.accumulatedValue;
                    calcState.currentOperator = currentKeyPressed;
                    calcState.accumulatedValue = 0.0;
                    calcState.isLastInputComplete = TRUE;
                    calcState.hasOperatorPending = TRUE;
                    calcState.currentSign = 1;
                    return;
                }

                calculationResult = performAdvancedCalculation(calcState.currentOperator, calcState.lastValue, calcState.accumulatedValue);
                calcState.accumulatedValue = calculationResult;

                if (calcState.operatorStackPointer == 0 || getTopOperator() == 0) {
                    break;
                }

                calcState.operatorStackPointer--;
                calcState.currentOperator = popOperator();
                calcState.lastValue = popOperand();
            } while (TRUE);
        }

        if (calcState.errorState == 0) {
            updateDisplay();
            calcState.lastValue = calcState.accumulatedValue;
            calcState.currentSign = 1;
            calcState.hasOperatorPending = TRUE;
            calcState.isLastInputComplete = TRUE;
            calcState.accumulatedValue = 0.0;
            calcState.currentOperator = currentKeyPressed;
        }
        else {
            calcState.lastValue = calcState.accumulatedValue;
            calcState.currentOperator = currentKeyPressed;
            calcState.accumulatedValue = 0.0;
            calcState.isLastInputComplete = TRUE;
            calcState.hasOperatorPending = TRUE;
            calcState.currentSign = 1;
        }
        return;
    }

    // Handle special cases
    handleSpecialCases(currentKeyPressed);

    updateDisplay();
}

/*
 * registerCalcClass
 *
 * This function registers the window class for the calculator application.
 * It sets up the window class attributes and registers it with the Windows system.
 *
 * The function performs the following tasks:
 * 1. Initializes a WNDCLASSEXA structure with zeroes
 * 2. Sets the window class style (CS_HREDRAW | CS_VREDRAW)
 * 3. Assigns the window procedure (calcWindowProc)
 * 4. Sets the application instance handle
 * 5. Loads default application icon and cursor
 * 6. Sets the window background color
 * 7. Assigns the class name from calcState
 * 8. Registers the window class with the Windows system
 *
 * @param appInstance     Handle to the current instance of the application
 * @return                The atom identifying the newly registered class, or 0 if registration fails
 */
ATOM registerCalcClass(HINSTANCE appInstance)
{
    WNDCLASSEXA wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = calcWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = appInstance;
    wcex.hIcon = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);
    wcex.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = calcState.className;
    wcex.hIconSm = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);

    return RegisterClassExA(&wcex);
}

BOOL CALLBACK ScientificDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        // Initialize scientific mode controls
        InitializeScientificControls(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_SIN:
        case IDC_BUTTON_COS:
        case IDC_BUTTON_TAN:
        case IDC_BUTTON_LOG:
        case IDC_BUTTON_LN:
        case IDC_BUTTON_EXP:
        case IDC_BUTTON_XY:
        case IDC_BUTTON_SQR:
        case IDC_BUTTON_CUBE:
        case IDC_BUTTON_FACT:
            // Handle scientific function buttons
            ProcessScientificFunction(LOWORD(wParam));
            updateDisplay();
            break;

        case IDC_RADIO_DEG:
        case IDC_RADIO_RAD:
        case IDC_RADIO_GRAD:
            // Handle angle mode selection
            SetAngleMode(LOWORD(wParam));
            break;

        case IDC_RADIO_HEX:
        case IDC_RADIO_DEC:
        case IDC_RADIO_OCT:
        case IDC_RADIO_BIN:
            // Handle number base selection
            SetNumberBase(LOWORD(wParam));
            updateDisplay();
            break;

        case IDCANCEL:
            // Close scientific mode
            SendMessage(calcState.windowHandle, WM_COMMAND, IDM_VIEW_STANDARD, 0);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        // Switch back to standard mode instead of closing
        SendMessage(calcState.windowHandle, WM_COMMAND, IDM_VIEW_STANDARD, 0);
        return TRUE;
    }

    return FALSE;
}

/*
 * resetCalculator
 *
 * This function performs a complete reset of the calculator, returning it to its initial state.
 * It resets all relevant fields in the _calculatorState structure.
 *
 * The function performs the following tasks:
 * 1. Resets all numeric values and registers
 * 2. Clears the error state
 * 3. Resets the calculator mode to standard
 * 4. Resets the number base to decimal (10)
 * 5. Clears the display and pending operations
 * 6. Resets UI-related fields
 * 7. Reinitializes string constants and paths
 * 8. Updates the UI elements
 *
 * This function ensures a clean slate for new calculations and resolves any
 * lingering issues from previous operations.
 *
 * No parameters.
 * No return value.
 */
void resetCalculatorState(void)
{
    // Reset numeric values
    memset(calcState.accumulatedValue, 0, sizeof(calcState.accumulatedValue));
    calcState.currentValueHighPart = 0;
    calcState.lastValue = 0;
    calcState.memoryRegister[0] = 0;
    calcState.memoryRegister[1] = 0;

    // Clear error state
    calcState.errorState = 0;
    calcState.errorCodeBase = 0;

    // Reset to standard mode and decimal base
    calcState.mode = STANDARD_MODE;
    calcState.numberBase = 10;

    // Reset key pressed and pending operations
    calcState.keyPressed = INVALID_BUTTON;
    calcState.hasOperatorPending = FALSE;
    calcState.isInputModeActive = FALSE;

    // Reset UI-related fields
    calcState.currentBackgroundColor = GetSysColor(COLOR_WINDOW);
    calcState.isHighContrastMode = FALSE;
    calcState.buttonHorizontalSpacing = BUTTON_BASE_SIZE;

    // Reset decimal separator
    calcState.decimalSeparator = '.';
    calcState.decimalSeparatorBuffer[0] = '.';
    calcState.decimalSeparatorBuffer[1] = '\0';

    // Reinitialize string constants and paths
    calcState.className = "CalculatorClass";
    calcState.registryKey = "SciCalc";
    calcState.modeText[STANDARD_MODE] = "Standard";
    calcState.modeText[SCIENTIFIC_MODE] = "Scientific";
    strcpy_s(calcState.helpFilePath, MAX_PATH, "calc.hlp");

    // Reset default precision
    calcState.defaultPrecisionValue = 0;

    // Update display
    updateDisplay();

    // Reset memory indicator
    SendMessage(calcState.windowHandle, WM_COMMAND, (WPARAM)MAKELONG(IDC_BUTTON_MC, 0), 0);

    // Reset radio buttons (assuming standard mode)
    CheckRadioButton(calcState.windowHandle, 0x7f, 0x81, 0x7f);

    // Update window
    InvalidateRect(calcState.windowHandle, NULL, TRUE);
    UpdateWindow(calcState.windowHandle);

    // Refresh the interface
    refreshInterface();
}

void toggleScientificMode(void)
{
    if (!calcState.isScientificModeActive)
    {
        calcState.isScientificModeActive = TRUE;
        calcState.scientificWindowHandle = CreateDialogParamA(calcState.appInstance,
            "SCIENTIFIC_DIALOG",
            calcState.windowHandle,
            ScientificDialogProc, 0);
        if (calcState.scientificWindowHandle == NULL)
        {
            // Handle error
            calcState.isScientificModeActive = FALSE;
        }
    }
    else
    {
        DestroyWindow(calcState.scientificWindowHandle);
        calcState.scientificWindowHandle = NULL;
        calcState.isScientificModeActive = FALSE;
    }

    // Update UI
    initColors(TRUE);
    refreshInterface();
}

void updateButtonState(uint buttonId, int state)
{
    uint* elementState = &elementStateTable;
    int buttonIndex = 0;
    int visibleButtonCount = 0;

    // Find the correct button in the element state table
    while (elementState < &END_OF_ELEMENT_STATE_TABLE) {
        UINT currentState = *elementState;
        if (((currentState >> 8 & 0xFF) == buttonId) && ((currentState & 3) != calculatorMode)) {
            break;
        }
        if ((currentState & 3) != calculatorMode) {
            visibleButtonCount++;
        }
        elementState++;
        buttonIndex++;
    }

    if (buttonIndex >= 0x3E) return; // Button not found

    HDC deviceContext = GetDC(mainCalculatorWindow);
    RECT buttonRect, clientRect;
    GetClientRect(mainCalculatorWindow, &clientRect);

    // Calculate button position and dimensions
    int buttonX, buttonY, buttonWidth, buttonHeight;
    int buttonsPerRow = *(int*)(&BUTTONS_PER_ROW + calculatorMode * 4);
    int totalButtons = *(int*)(&DAT_0040c038 + calculatorMode * 4);

    if (visibleButtonCount < totalButtons - 3) {
        // Main buttons
        buttonWidth = BUTTON_BASE_SIZE;
        int row = visibleButtonCount / buttonsPerRow;
        int col = visibleButtonCount % buttonsPerRow;
        buttonX = row * (BUTTON_BASE_SIZE + 4) + calculateButtonHorizontalOffset(buttonIndex) + 6 + VERTICAL_OFFSET;
        buttonY = col * 17 + (calculatorMode == 0 ? -13 : 0) + *(int*)(&BUTTON_COLUMN_CONFIG + calculatorMode * 4);
    }
    else {
        // Special buttons
        buttonWidth = (BUTTON_BASE_SIZE * 4) / 3 + 1;
        buttonX = clientRect.right - ((totalButtons - visibleButtonCount) - 3) * (buttonWidth + 4) - (calculatorMode == 0 ? 1 : 0) - 7;
        if (visibleButtonCount == 0x53) {
            buttonX -= (calculatorMode == 0 ? 2 : 1);
        }
        buttonY = (calculatorMode == 0 ? -12 : 0) + *(int*)(&BUTTON_ROW_CONFIG + calculatorMode * 4);
    }

    buttonRect.left = buttonX;
    buttonRect.top = (buttonY * BUTTON_SCALING_FACTOR + 7) >> 3;
    buttonRect.right = buttonX + buttonWidth;
    buttonRect.bottom = ((buttonY + BUTTON_VERTICAL_SPACING) * BUTTON_SCALING_FACTOR + 7) >> 3;

    // Draw button based on state
    switch (state) {
    case 100: // Click animation
        DrawFrameControl(deviceContext, &buttonRect, DFC_BUTTON, DFCS_PUSHED);
        Sleep(10);
        DrawFrameControl(deviceContext, &buttonRect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
        Sleep(10);
        DrawFrameControl(deviceContext, &buttonRect, DFC_BUTTON, DFCS_BUTTONPUSH);
        break;
    case 0x65: // Pressed
        DrawFrameControl(deviceContext, &buttonRect, DFC_BUTTON, DFCS_PUSHED);
        buttonRect.left++; buttonRect.top++;
        break;
    case 0x66: // Normal
        DrawFrameControl(deviceContext, &buttonRect, DFC_BUTTON, DFCS_BUTTONPUSH);
        break;
    }

    // Draw button text
    const char* buttonText = (&lpString)[buttonIndex];
    int textLength = lstrlenA(buttonText);

    if (isHighContrastMode) { // High contrast mode
        COLORREF textColor = GetSysColor(COLOR_BTNTEXT);
        COLORREF backgroundColor = GetSysColor(COLOR_BTNFACE);
        SetTextColor(deviceContext, getElementColor(buttonIndex, backgroundColor, textColor));
    }

    SetBkMode(deviceContext, TRANSPARENT);
    SIZE textSize;
    GetTextExtentPointA(deviceContext, buttonText, textLength, &textSize);
    int textX = (buttonWidth + (buttonX * 2 - textSize.cx)) / 2;
    int textY = ((buttonY + 3) * BUTTON_SCALING_FACTOR * 2 + 15) >> 4;
    TextOutA(deviceContext, textX, textY, buttonText, textLength);
}

void updateDecimalSeparator()
{
    int separatorPosition;

    separatorPosition = calcState.decimalSeparatorBuffer[0];
    if (separatorPosition == 0) {
        if (calcState.currentValueHighPart == 0) {
            separatorPosition = 2;
        }
        else {
            separatorPosition = calcState.currentValueHighPart + 1;
        }
    }
    calcState.decimalSeparator = calcState.decimalSeparatorBuffer[0];
    calcState.decimalSeparatorBuffer[1] = '\0';
}

void updateDisplay(void)
{
    char displayBuffer[64];
    char* resultString;

    if (!isInputModeActive) {
        if (numberBase == 10) {
            formatNumberForDisplay(displayBuffer, _accumulatedValue, 13);
            if (isScientificNotationEnabled && !hasDecimalPart) {
                formatScientificNotation(displayBuffer, &displayResultBuffer);
            }
            else {
                formatFloatAutomatically(displayBuffer, &displayResultBuffer);
            }
            resultString = displayResultBuffer;
        }
        else {
            float10 formattedValue = processFloatingPointForDisplay(accumulatedValue, currentValueHighPart);
            _accumulatedValue = (double)formattedValue;

            if (ABS(formattedValue) > 4294967295.0) {
                int errorCode = (_accumulatedValue < 0.0) ? 4 : 3;
                handleCalculationError(errorCode);
                return;
            }

            long long integerValue = __ftol();
            intToBaseString(integerValue & _DAT_0040c0fc, displayResultBuffer, numberBase);
            CharUpperA(displayResultBuffer);
            resultString = displayResultBuffer;
        }
    }
    else {
        resultString = (numberBase == 10) ? (char*)&DAT_0040b180 : displayResultBuffer;
    }

    SetDlgItemTextA(mainCalculatorWindow, calculatorMode + 0x19d, resultString);
}


/*
 * statisticsWindowProc
 *
 * This is the dialog box procedure for the statistics window. It handles
 * messages for the window, including button clicks, closing the window,
 * and potentially other events. It interacts with the main calculator
 * state (`calcState`) and the statistics data structure.
 *
 * @param window        Handle to the statistics window dialog box.
 * @param message     The message identifier.
 * @param wParam       Additional message-specific information.
 * @param lParam       Additional message-specific information.
 * @return            TRUE if the message was processed, FALSE otherwise.
 */
BOOL CALLBACK statisticsWindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hwndStatisticsDisplay;
    static int  selectedIndex = -1;
    static char selectedDataPointStr[MAX_DISPLAY_DIGITS] = { 0 };
    static int dataPointCount = 0;

    switch (message) {
        case WM_INITDIALOG: {
            // Get the handle to the statistics display listbox
            hwndStatisticsDisplay = GetDlgItem(windowHandle, ID_STAT_BUTTON);

            // Initialize statistics display
            updateStatisticsDisplay(windowHandle);
            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case IDC_BUTTON_STAT_RED: {
                // Copy selected data point to calculator
                if (selectedIndex != -1) {
                    strcpy(calcState.accumulatedValue, selectedDataPointStr);
                    updateDisplay(); // Update the main calculator display
                }
                else {
                    MessageBeep(0); // No item selected
                }
                break;
            }

            case IDC_BUTTON_STAT_LOAD: {
                // Load data from clipboard
                if (OpenClipboard(windowHandle)) {
                    HANDLE clipboardData = GetClipboardData(CF_TEXT);
                    if (clipboardData != NULL) {
                        char* clipboardText = (char*)GlobalLock(clipboardData);
                        parseAndStoreDataPoints(clipboardText); // Parse and store clipboard data
                        GlobalUnlock(clipboardData);
                        updateStatisticsDisplay(windowHandle);
                    }
                    CloseClipboard();
                }
                break;
            }

            case IDC_BUTTON_STAT_CE:
                // Clear last entry
                if (dataPointCount > 0) {
                    dataPointCount--;
                    // Remove the last item from the display
                    SendMessage(hwndStatisticsDisplay, LB_DELETESTRING, dataPointCount, 0);
                    updateStatisticsDisplay(windowHandle);
                }
                break;

            case IDC_BUTTON_STAT_CAD:
                // Clear all data
                dataPointCount = 0;
                // Clear the statistics display
                SendMessage(hwndStatisticsDisplay, LB_RESETCONTENT, 0, 0);
                updateStatisticsDisplay(windowHandle);
                break;
            }
            return TRUE;

        case WM_CLOSE: {
            calcState.statisticsWindowOpen = FALSE;
            DestroyWindow(windowHandle);
            return TRUE;
        }

        case WM_LBUTTONDOWN: {
            // Handle listbox item selection
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            selectedIndex = SendMessage(hwndStatisticsDisplay, LB_ITEMFROMPOINT, 0, MAKELONG(xPos, yPos));
            if (selectedIndex != LB_ERR && selectedIndex < dataPointCount) {
                // Get the text of the selected item
                SendMessage(hwndStatisticsDisplay, LB_GETTEXT, selectedIndex, (LPARAM)selectedDataPointStr);
            }
            else {
                selectedIndex = -1;  // No item selected
            }
            break;
        }
    }

    return FALSE;
}

/*
 * WinMain
 *
 * This function serves as the Windows entry point for the calculator application.
 * It initializes the application, creates the main window, and runs the message loop
 * until the application is closed.
 *
 * The function performs the following tasks:
 * 1. Initializes the calculator state by calling initCalcState()
 * 2. Registers the calculator window class using registerCalcClass()
 * 3. Creates the main calculator window using initInstance()
 * 4. Enters the main message loop to process and dispatch Windows messages
 *
 * @param appInstance     Handle to the current instance of the application
 * @param unused          Always NULL for Win32 applications (legacy parameter)
 * @param commandLine     Command line arguments as a single string
 * @param windowMode      Controls how the window is to be shown (e.g., maximized)
 * @return                The value from the WM_QUIT message's wParam parameter
 */
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE unused, LPSTR commandLine, int windowMode)
{
    MSG msg;
    appInstance = calcState.appInstance;

    // Initialize calculator state
    initCalcState();

    if (!registerCalcClass(calcState.appInstance))
    {
        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (!initInstance(calcState.appInstance, windowMode))
    {
        MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}