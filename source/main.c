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

#include <windows.h>
#include "main.h"

_calculatorWindows calcWindows = {
    .main = NULL,
    .scientific = NULL
};

_calculatorState calcState;
_calculatorMode calcMode = STANDARD_MODE;

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
    0x00 | IDC_BUTTON_C,           // Standard Mode | Clear All

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

void initCalculatorState(void)
{
    // Initialize string constants
    calcState.className = "CalculatorClass";
    calcState.registryKey = "SciCalc";
    calcState.modeText[STANDARD_MODE] = "Standard";
    calcState.modeText[SCIENTIFIC_MODE] = "Scientific";

    // Set default help file path
    strcpy_s(calcState.helpFilePath, MAX_PATH, "calc.hlp");

    // Initialize numeric values
    calcState.defaultPrecisionValue = 0;
    calcState.errorCodeBase = 0;
    calcState.appInstance = NULL;
    calcState.pressedButton = INVALID_BUTTON;
    calcState.windowHandle = NULL;
    calcState.mode = STANDARD_MODE;
    calcState.currentValueHighPart = 0;
    calcState.accumulatedValue = 0;
    calcState.lastValue = 0;
    calcState.errorState = 0;
    calcState.currentKeyPressed = 0;
    calcState.decimalSeparator = DEFAULT_DECIMAL_SEPARATOR;
    calcState.memoryRegister[0] = 0;
    calcState.memoryRegister[1] = 0;

    updateDecimalSeparator();

    // Additional initialization steps
    //InitializeRandomSeed(GetTickCount());  // Seed the random number generator
    //setNumberFormatting();  // Set up number formatting functions
    //configureFPUPrecision();  // Configure floating-point unit precision
}

int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE unused, LPSTR commandLine, int windowMode)
{
    MSG msg;
    appInstance = calcState.appInstance;

    // Initialize calculator state
    initCalculatorState(&calcState);

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

bool initInstance(HINSTANCE appInstance, int windowMode)
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
        return false;
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

    return true;
}

void initColors(int forceUpdate)
{
    char previousDecimalSeparator;
    DWORD backgroundColor;
    BOOL isIconic;
    int windowHeight, windowWidth;
    HMENU hMenu;
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
            hMenu = GetMenu(calcWindows.main);
            hMenu = GetSubMenu(hMenu, 1);
            CheckMenuItem(hMenu, calcState.mode, MF_BYCOMMAND | MF_CHECKED);
            CheckMenuItem(hMenu, 1 - calcState.mode, MF_BYCOMMAND | MF_UNCHECKED);

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

void handleCalculationError(int errorCode)
{
    LPCSTR errorMessage = getStatusCode(errorCode);
    if (errorMessage != NULL) {
        MessageBoxA(NULL, errorMessage, "Runtime Error", MB_ICONERROR);
    }
}

// This function retrieves a status message corresponding to a given status code.
// It searches through the STATUS_MESSAGE_TABLE and returns the associated message.
// If no matching status code is found, it returns NULL.
const char* getStatusCode(int statusCode)
{
    // Iterate through the STATUS_MESSAGE_TABLE
    for (int i = 0; i < (STATUS_MESSAGE_TABLE_END - STATUS_MESSAGE_TABLE); i++) {
        // Check if the current table entry matches the given status code
        if (STATUS_MESSAGE_TABLE[i].status == statusCode) {
            // If a match is found, return the corresponding message
            return STATUS_MESSAGE_TABLE[i].message;
        }
    }
    // If no matching status code is found, return NULL
    return NULL;
}

void refreshInterface(void)
{
    PAINTSTRUCT ps;
    RECT clientRect, buttonRect, edgeRect;
    SIZE textSize;
    HGDIOBJ oldFont, oldBrush;
    COLORREF backgroundColor, textColor;
    HDC hdc;
    int row, col, buttonIndex;
    int buttonWidth, buttonHeight, buttonSpacing;
    int textLength, textX, textY;
    char* buttonText;

    // Set up colors
    backgroundColor = GetSysColor(COLOR_BTNFACE);
    textColor = GetSysColor(COLOR_BTNTEXT);

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
    for (row = 0; row < CALC_ROWS; row++) {
        for (col = 0; col < CALC_COLS; col++) {
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

                if (isHighContrastMode()) {
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
}

void processButtonClick(uint keyPressed)
{
    bool isValidInput;
    double calculationResult;
    char tempCharBuffer[52];
    int tempBaseValue;

    // Check if the key is a special function key
    if (isSpecialFunctionKey(keyPressed)) {
        previousKeyPressed = currentKeyPressed;
        currentKeyPressed = keyPressed;
    }

    // Handle error state
    if (errorState != 0 && !isClearKey(keyPressed)) {
        MessageBeep(0);
        return;
    }

    // Handle input mode activation
    if (!isInputModeActive) {
        isValidInput = isNumericInput(keyPressed) || keyPressed == 0x55;
        if (isValidInput) {
            isInputModeActive = true;
            InitcalcState(&calcState);
        }
    }
    else if (isOperatorKey(keyPressed) || keyPressed == 0x12d) {
        isInputModeActive = false;
    }

    // Reset calculator state for certain key combinations
    if (isNumericInput(keyPressed) &&
        (isPreviousKeyOperator() || (previousKeyPressed == 0x29 && parenthesisDepth == 0) || keyPressed == 0x12d)) {
        resetCalculatorState();
    }

    // Process numeric input
    if (isNumericInput(keyPressed)) {
        int digit = convertKeyToDigit(keyPressed);
        if (digit < numberBase) {
            if (numberBase == 10) {
                if (!appendDigit(&calculatorState, digit)) {
                    MessageBeep(0);
                    return;
                }
            }
            else {
                if (!appendDigitInBase(&calculatorState, digit, numberBase)) {
                    MessageBeep(0);
                    return;
                }
            }
        }
    }
    // Process operator input
    else if (isOperatorKey(keyPressed)) {
        if (hasOperatorPending) {
            calculationResult = performAdvancedCalculation(currentOperator, lastValue, calcState.currentValue);
            updatecalcState(calculationResult);

            while (operatorStackPointer > 0 && shouldProcessPendingOperator()) {
                operatorStackPointer--;
                currentOperator = popOperator();
                lastValue = popOperand();
                calculationResult = performAdvancedCalculation(currentOperator, lastValue, calcState.currentValue);
                updateCalculatorState(calculationResult);
            }
        }

        if (errorState == 0) {
            updateDisplay();
            prepareForNextOperation(keyPressed);
        }
        else {
            handleErrorState(keyPressed);
        }
    }
    // Handle special cases (scientific mode, parentheses, etc.)
    else {
        handleSpecialCases(keyPressed);
    }

    updateDisplay();
}

bool handleContextHelp(HWND hwnd, HINSTANCE hInstance, UINT message)
{
    // ID do recurso do menu de contexto
    const UINT CONTEXT_MENU_RESOURCE_ID = 4;

    // Flags para o TrackPopupMenuEx
    const UINT TRACK_POPUP_FLAGS = TPM_RETURNCMD | TPM_RIGHTBUTTON;

    // Carrega o menu de contexto dos recursos da aplica��o
    HMENU hMenu = LoadMenuA(hInstance, MAKEINTRESOURCE(CONTEXT_MENU_RESOURCE_ID));
    if (hMenu == NULL) {
        // Falha ao carregar o menu
        return false;
    }

    // Obt�m o primeiro (e provavelmente �nico) submenu
    HMENU hPopupMenu = GetSubMenu(hMenu, 0);
    if (hPopupMenu == NULL) {
        // Falha ao obter o submenu
        DestroyMenu(hMenu);
        return false;
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
    DestroyMenu(hMenu);

    // Verifica se o item "What's This?" foi selecionado (ID 8)
    return (result == 8);
}

DWORD getCalculatorButton(ushort mouseX, ushort mouseY)
{
    int buttonWidth = (BUTTON_BASE_SIZE * 4) / 3 + 5;
    int horizontalPosition = 0;
    int rowIndex = (BUTTON_BASE_SIZE * 4) / 3 + 1;
    int modeOffset = calcMode * 4;

    // Check if the click is in the main button area
    int topEdge = ((BUTTON_ROW_CONFIG + BUTTON_VERTICAL_SPACING) * BUTTON_SCALING_FACTOR + 7) >> 3;
    int bottomEdge = (BUTTON_ROW_CONFIG * BUTTON_SCALING_FACTOR + 7) >> 3;

    if (mouseY >= topEdge && mouseY < bottomEdge) {
        int verticalPosition = VERTICAL_OFFSET + 6;
        int leftEdge = (BUTTON_COLUMN_CONFIG * BUTTON_SCALING_FACTOR + 7) >> 3;
        int rightEdge = ((BUTTON_COLUMN_CONFIG + 0x56) * BUTTON_SCALING_FACTOR + 7) >> 3;

        if (mouseX >= leftEdge && mouseX <= rightEdge) {
            // Find the column of the clicked button
            int column = 0;
            bool buttonFound = false;
            int buttonsPerRow = BUTTONS_PER_ROW[calcMode];

            while (column < buttonsPerRow && !buttonFound) {
                if (mouseY >= leftEdge + column * ((17 * BUTTON_SCALING_FACTOR + 7) >> 3) &&
                    mouseY <= leftEdge + column * ((17 * BUTTON_SCALING_FACTOR + 7) >> 3) + ((14 * BUTTON_SCALING_FACTOR + 7) >> 3)) {
                    buttonFound = true;
                }
                column++;
            }

            if (buttonFound && column <= buttonsPerRow) {
                // Find the row of the clicked button
                int row = 0;
                bool rowFound = false;
                int buttonCount = BUTTON_COUNT_PER_MODE[calcMode];

                while (row < buttonCount && !rowFound) {
                    horizontalPosition = adjustButtonHorizontalPosition(horizontalPosition, row, 0);
                    if (mouseX >= horizontalPosition + verticalPosition &&
                        mouseX <= horizontalPosition + verticalPosition + BUTTON_BASE_SIZE) {
                        rowFound = true;
                    }
                    horizontalPosition += BUTTON_BASE_SIZE + 4;
                    row++;
                }

                if (rowFound) {
                    // Calculate button index and return button ID
                    int buttonIndex = buttonsPerRow * (row - 1) + column - 1;
                    int elementIndex = 0;
                    uint* statePtr = &elementStateTable;

                    while (buttonIndex >= 0 && statePtr < &END_OF_ELEMENT_STATE_TABLE) {
                        if ((*statePtr & 3) != calculatorMode) {
                            buttonIndex--;
                        }
                        statePtr++;
                        elementIndex++;
                    }

                    byte rawButtonId = ((byte*)BUTTON_ID_MAP)[elementIndex * 4];
                    buttonId = (uint)rawButtonId;
                    return buttonId;
                }
            }
        }
    }
    else {
        // Check for special buttons at the top
        RECT clientRect;
        GetClientRect(mainCalculatorWindow, &clientRect);

        for (int i = 0; i < 3; i++) {
            if (mouseX <= clientRect.right - horizontalPosition - (calculatorMode == 0 ? 1 : 0) - 10 &&
                mouseX > clientRect.right - horizontalPosition - rowIndex - (calculatorMode == 0 ? 1 : 0) - 10) {
                return i + SPECIAL_BUTTON_OFFSET;
            }
            horizontalPosition += buttonWidth;
        }
    }

    return 0;  // No button found
}

LRESULT CALLBACK calcWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    int iVar;
    BOOL bVar;
    HMENU hMenu;
    DWORD DVar;
    UINT commandID;
    POINT ptMouse;
    UINT menuItemID;
    WORD mouseX, mouseY;
    static BOOL isButtonPressed = FALSE;

    switch (uMsg)
    {
    case WM_ACTIVATE:
        iVar = (wParam == WA_ACTIVE) ? SW_SHOW : SW_HIDE;
        if (scientificModeWindow != NULL)
        {
            ShowWindow(scientificModeWindow, iVar);
        }
        break;

    case WM_DESTROY:
        WinHelpA(mainCalculatorWindow, helpFilePath, HELP_QUIT, 0);
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
        DestroyWindow(mainCalculatorWindow);
        break;

    case WM_HELP:
        commandID = 0;
        if (hWnd == (HWND)wParam)
        {
            ptMouse.x = GET_X_LPARAM(lParam);
            ptMouse.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &ptMouse);
            commandID = getCalculatorButton((WORD)ptMouse.x, (WORD)ptMouse.y);
        }
        if (commandID == 0)
        {
            WinHelpA((HWND)wParam, helpFilePath, HELP_WM_HELP, HELP_CONTEXT_DATA);
            return 0;
        }
        bVar = handleContextHelp(mainCalculatorWindow, g_hInstance, lParam);
        if (bVar)
        {
            if (commandID == STAT_BUTTON_ID && calculatorMode == 0)
            {
                commandID = STAT_ALT_BUTTON_ID;
            }
            if (commandID > MEMORY_BUTTON_START && commandID < MEMORY_BUTTON_END)
            {
                commandID = MEMORY_BUTTON_DEFAULT;
            }
            if (commandID > DIGIT_BUTTON_START && commandID < DIGIT_BUTTON_END)
            {
                commandID = DIGIT_BUTTON_DEFAULT;
            }
            WinHelpA((HWND)wParam, helpFilePath, HELP_CONTEXTMENU, commandID);
            return 0;
        }
        break;

    case WM_COMMAND:
        commandID = LOWORD(wParam);
        if (HIWORD(wParam) == 1 && commandID < MAX_COMMAND_ID)
        {
            if (commandID == ID_STAT_BUTTON && calculatorMode == 1)
            {
                commandID = ID_STAT_BUTTON_ALT;
            }
            for (int i = 0; i < 0x3d; i++)
            {
                if ((elementStateTable[i] >> 8 & 0xff) == commandID &&
                    (elementStateTable[i] & 3) != calculatorMode)
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
        hMenu = GetMenu(hWnd);
        EnableMenuItem(hMenu, menuItemID, commandID);
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
        if (g_nPressedButton != INVALID_BUTTON)
        {
            commandID = getCalculatorButton(mouseX, mouseY);
            if (commandID == g_nPressedButton || isButtonPressed)
            {
                if (commandID == g_nPressedButton && isButtonPressed)
                {
                    updateButtonState(g_nPressedButton, BUTTON_STATE_DOWN);
                    isButtonPressed = FALSE;
                }
            }
            else
            {
                updateButtonState(g_nPressedButton, BUTTON_STATE_UP);
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
            g_nPressedButton = commandID;
            updateButtonState(commandID, BUTTON_STATE_DOWN);
            isButtonPressed = FALSE;
            SetCapture(mainCalculatorWindow);
        }
        break;

    case WM_LBUTTONUP:
        ReleaseCapture();
        mouseX = LOWORD(lParam);
        mouseY = HIWORD(lParam);
        commandID = getCalculatorButton(mouseX, mouseY);
        if (commandID == g_nPressedButton)
        {
            if (commandID != 0)
            {
                updateButtonState(commandID, BUTTON_STATE_UP);
                isButtonPressed = TRUE;
                processButtonClick(commandID);
            }
        }
        g_nPressedButton = INVALID_BUTTON;
        break;

    default:
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }

    return result;
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

void adjustMemoryAllocation(void)
{
    if (isScientificMode() && currentPrecisionLevel < MAX_STANDARD_PRECISION) {
        currentAllocationSize = INITIAL_MEMORY_SIZE;
        minimumAllocationSize = EXTENDED_MEMORY_SIZE;
    }
}

void initStandardStreams(void)
{
    STARTUPINFOA startupInfo;
    UINT redirectionSize, tempValue;
    UINT* sourcePtr, * flagPtr;
    DWORD streamType;
    HANDLE fileHandle;
    int streamIndex;
    HANDLE* handlePtr;
    UINT* handleData;

    GetStartupInfoA(&startupInfo);

    // Check for stream redirection information
    if (startupInfo.lpReserved2 != NULL) {
        redirectionSize = *(UINT*)startupInfo.lpReserved2;
        tempValue = (redirectionSize > MAX_STREAM_REDIRECTION_BUFFER_SIZE) ?
            MAX_STREAM_REDIRECTION_BUFFER_SIZE : redirectionSize;

        // Copy stream flags
        flagPtr = (UINT*)&standardStreamFlags;
        sourcePtr = (UINT*)startupInfo.lpReserved2;
        for (UINT i = tempValue >> 2; i > 0; i--) {
            *flagPtr++ = *sourcePtr++;
        }
        for (tempValue &= 3; tempValue > 0; tempValue--) {
            *(BYTE*)flagPtr++ = *(BYTE*)sourcePtr++;
        }

        // Copy stream handles
        tempValue = (redirectionSize > MAX_STREAM_REDIRECTION_BUFFER_SIZE) ?
            MAX_STREAM_REDIRECTION_BUFFER_SIZE : redirectionSize;
        handleData = (UINT*)(redirectionSize + 4 + (INT)startupInfo.lpReserved2);
        UINT* handlePtr = (UINT*)&streamHandles;
        for (tempValue &= 0xFFFFFFFC; tempValue > 0; tempValue--) {
            *handlePtr++ = *handleData++;
        }
    }

    // Initialize standard streams
    streamIndex = 0;
    handlePtr = (HANDLE*)&streamHandles;
    do {
        if (*handlePtr == INVALID_HANDLE_VALUE) {
            streamType = (streamIndex == 0) ? STD_INPUT_HANDLE :
                (streamIndex == 1) ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE;
            fileHandle = GetStdHandle(streamType);
            *handlePtr = fileHandle;
            if (fileHandle != INVALID_HANDLE_VALUE) {
                standardStreamFlags[streamIndex] = 0x81;  // Valid and open stream
                streamType = GetFileType(fileHandle);
                if ((streamType & 0x0003) == FILE_TYPE_CHAR) {
                    standardStreamFlags[streamIndex] |= 0x40;  // Console
                }
                else if ((streamType & 0x0003) == FILE_TYPE_PIPE) {
                    standardStreamFlags[streamIndex] |= 0x08;  // Pipe
                }
            }
        }
        else {
            standardStreamFlags[streamIndex] |= 0x80;  // Already open
        }
        handlePtr++;
        streamIndex++;
    } while (handlePtr < (HANDLE*)&streamHandles[3]);
}

void initApplicationCodePage(void)
{
    configureCodePageSettings(SYSTEM_CODE_PAGE);
}

DWORD configureCodePageSettings(int requestedCodePage)
{
    DWORD activeCodePage = setupCodePage(requestedCodePage);
    if (currentCodePage == activeCodePage) {
        return 0;  // No change needed
    }

    if (activeCodePage != 0) {
        // Check if the active code page is in the supported list
        for (int i = 0; i < NUM_SUPPORTED_CODEPAGES; i++) {
            if (supportedCodePages[i] == activeCodePage) {
                // Initialize character type flags
                memset(charTypeFlags, 0, sizeof(charTypeFlags));

                // Set up character ranges for the code page
                for (int flagIndex = 0; flagIndex < 4; flagIndex++) {
                    byte* charRangePtr = &charRangeTable[i * 6 + flagIndex * 8];
                    while (charRangePtr[0] != 0 && charRangePtr[1] != 0) {
                        for (DWORD charIndex = charRangePtr[0]; charIndex <= charRangePtr[1]; charIndex++) {
                            charTypeFlags[charIndex] |= 1 << flagIndex;
                        }
                        charRangePtr += 2;
                    }
                }

                // Set up code page specific settings
                currentCodePage = activeCodePage;
                _codePageSpecificFlag = getPageSpecificFlag(activeCodePage);
                customCharTypeFlag1 = customCharTypeTable1[i];
                customCharTypeFlag2 = customCharTypeTable2[i];
                customCharTypeFlag3 = customCharTypeTable3[i];
                return 0;
            }
        }

        // If not in the supported list, try to get CP info
        CPINFO codepageInfo;
        if (GetCPInfo(activeCodePage, &codepageInfo)) {
            memset(charTypeFlags, 0, sizeof(charTypeFlags));

            if (codepageInfo.MaxCharSize >= 2) {
                // Set up lead byte ranges
                for (BYTE* leadBytePtr = codepageInfo.LeadByte; leadBytePtr[0] && leadBytePtr[1]; leadBytePtr += 2) {
                    for (DWORD i = leadBytePtr[0]; i <= leadBytePtr[1]; i++) {
                        charTypeFlags[i] |= 4;  // Mark as lead byte
                    }
                }

                // Mark single-byte characters
                for (uint i = 1; i < 0xFF; i++) {
                    charTypeFlags[i] |= 8;
                }

                currentCodePage = activeCodePage;
                _codePageSpecificFlag = getPageSpecificFlag(activeCodePage);
            }
            else {
                _codePageSpecificFlag = 0;
                currentCodePage = 0;
            }

            customCharTypeFlag1 = customCharTypeFlag2 = customCharTypeFlag3 = 0;
            return 0;
        }

        if (!isCustomCodePage) {
            return ERROR_CODE;
        }
    }

    resetCharacterTypeFlags();
    return 0;
}

void initApplicationPath(void)
{
    char** pathComponents;
    char* appPathBuffer;
    int pathDataSize;
    int componentCount;

    // Get the full path of the current executable (max 260 characters)
    GetModuleFileNameA((HMODULE)0x0, &g_appPath, 0x104);
    g_appPathPtr = &g_appPath;
    appPathBuffer = &g_appPath;

    // If endOfMemoryTable is not empty, use it instead of the retrieved path
    if (*endOfMemoryTable != '\0') {
        appPathBuffer = endOfMemoryTable;
    }

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

    // Store results in global variables
    g_pathComponentsPtr = pathComponents;
    g_pathComponentCount = componentCount - 1;
}

void initEnvironmentVariables(void)
{
    int* envVarArray;
    size_t stringLength;
    size_t copyLength;
    size_t remainingLength;
    char* envString;
    int envVarCount;
    char* nextPos;
    char* currentEnvVar;
    char* sourcePtr;
    char* destPtr;
    char currentChar;

    // Count the number of environment variables
    envVarCount = 0;
    envString = envStringStart;
    while (*envString != '\0') {
        if (*envString != '=') {
            envVarCount++;
        }
        // Move to next environment string
        while (*envString++ != '\0');
    }

    // Allocate memory for environment variable pointers
    envVarArray = (int*)allocateMemoryFromHeap((envVarCount + 1) * sizeof(int));
    g_environmentVariables = envVarArray;
    if (envVarArray == NULL) {
        showRunTimeError(9);  // Memory allocation error
        return;
    }

    // Process each environment variable
    envString = envStringStart;
    while (*envString != '\0') {
        if (*envString != '=') {
            // Calculate length of the environment string
            stringLength = strlen(envString);

            // Allocate memory for the environment string
            *envVarArray = (int)allocateMemoryFromHeap(stringLength + 1);
            if (*envVarArray == 0) {
                showRunTimeError(9);  // Memory allocation error
                return;
            }

            // Copy the environment string
            memcpy((char*)*envVarArray, envString, stringLength + 1);
            envVarArray++;
        }
        // Move to next environment string
        envString += stringLength + 1;
    }

    // Null-terminate the environment variable array
    *envVarArray = 0;
}

DWORD initCalculatorRuntime(int initializationFlags)
{
    int result;

    // Call the calculator environment initialization function if it exists
    if (g_initializeCalculatorEnvironmentPtr != NULL) {
        (*g_initializeCalculatorEnvironmentPtr)();
    }

    // Initialize functions without return values
    __initterm(&CALCULATOR_INIT_FUNCTIONS_START, &CALCULATION_INIT_FUNCTIONS_END);

    // Initialize functions with return values and store the result
    result = __initterm(&CALCULATOR_INIT_FUNCTIONS_WITH_RETURN_START,
        &CALCULATOR_INIT_FUNCTIONS_WITH_RETURN_END);

    return result;
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