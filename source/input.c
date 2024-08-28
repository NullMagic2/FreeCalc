/*-----------------------------------------------------------------------------
    input.c --  Input Handling Functions for the Windows Calculator
                    (reconstructed code).

                This module handles the interpretation of keyboard and mouse
                input for the calculator, converting user actions into
                calculator operations, digit entries, and special functions.
                It manages input modes and interacts with other modules
                to update the calculator's state and display.

                Key functions include:

                - appendDigit: Appends a digit to the accumulated value,
                                handling different number bases.
                - convertKeyToDigit: Maps button IDs to numeric digit values.
                - isClearKey: Determines if a key is a clear (CE or C) key.
                - isNumericInput:  Identifies numeric input keys (0-9, A-F).
                - isOperatorKey:  Identifies operator keys (+, -, *, /, etc.).
                - isPreviousKeyOperator: Checks if the last key was an operator.
                - isSpecialFunctionKey:  Determines if a key is a special
                                        function (memory, mode, etc.).
                - updateInputMode: Activates/deactivates input mode based on
                                    the key pressed.

-------------------------------------------------------------------------------*/
#include "..//headers//input.h"
#include "..//headers//main.h"

extern _calculatorState calcState;

#ifndef LONGLONG_MAX
#define LONGLONG_MAX 0x7FFFFFFFFFFFFFFFLL
#endif

/*
 * appendDigit
 *
 * This function appends a digit to the current input value in the calculator.
 * It handles both integer and decimal parts of the number, updating the
 * internal state represented by the byte array passed as the first parameter.
 * The function supports multiple number bases, including decimal (base 10),
 * hexadecimal (base 16), octal (base 8), and binary (base 2).
 *
 * For non-decimal bases, digits are interpreted as follows:
 * - Decimal: '0' through '9'
 * - Hexadecimal: '0' through '9' and 'A' through 'F' (case-insensitive)
 * - Octal: '0' through '7'
 * - Binary: '0' and '1'
 *
 * The function handles leading zeros, maximum digit limits for each base,
 * and placement of the decimal separator for floating-point numbers.
 *
 * @param accumulatedValue Pointer to the byte array representing the calculator's internal state
 * @param digit         The digit to be appended (0-15, depending on the current base)
 * @return              1 if the digit was successfully appended, 0 otherwise
 */


BOOL appendDigit(char* accumulatedValue, int digit)
{
    int integerDigits = strlen(accumulatedValue);
    char digitChar;

    // Handle digits 0-9 and A-F for hexadecimal
    if (digit < 10) {
        digitChar = digit + '0';
    }
    else if (digit < 16) {
        digitChar = digit - 10 + 'A';
    }
    else {
        return FALSE;  // Invalid digit for any supported base
    }

    // Check if the digit is valid for the current base
    if (digit >= calcState.numberBase) {
        return FALSE;
    }

    if (!hasDecimalSeparator(accumulatedValue)) {
        // Integer part
        if (digit == 0 && integerDigits == 0) {
            return TRUE;  // Ignore leading zeros
        }

        switch (calcState.numberBase) {
        case 2:  // Binary
            if (integerDigits >= MAX_BINARY_DIGITS) return FALSE;
            break;
        case 8:  // Octal
            if (integerDigits >= MAX_OCTAL_DIGITS) return FALSE;
            break;
        case 10: // Decimal
            if (integerDigits >= MAX_DECIMAL_DIGITS) return FALSE;
            break;
        case 16: // Hexadecimal
            if (integerDigits >= MAX_HEXADECIMAL_DIGITS) return FALSE;
            break;
        default:
            return FALSE; // Invalid base
        }

        accumulatedValue[integerDigits] = digitChar;
        accumulatedValue[integerDigits + 1] = '\0';

        if (hasDecimalSeparator(accumulatedValue)) {
            accumulatedValue[integerDigits + 1] = calcState.decimalSeparator;
            accumulatedValue[integerDigits + 2] = '\0';
        }
    }
    else {
        // Decimal part (only for base 10)
        if (calcState.numberBase != 10) {
            return FALSE;
        }
        int decimalDigits = calcState.currentValueHighPart;
        if (decimalDigits >= MAX_DECIMAL_DIGITS) {
            return FALSE;
        }
        calcState.currentValueHighPart = decimalDigits * 10 + digit;

        // Shift existing decimal digits
        int insertPos = integerDigits + decimalDigits + 1;
        memmove(&accumulatedValue[insertPos + 1], &accumulatedValue[insertPos], strlen(&accumulatedValue[insertPos]) + 1);
        accumulatedValue[insertPos] = digitChar;
    }

    DecimalToFloat(&calcState);
    return TRUE;
}

/*
 * convertKeyToDigit
 *
 * This function converts a key code representing a calculator button
 * to its corresponding digit value. It supports digits 0-9 for decimal
 * input and A-F for hexadecimal input.
 *
 * The function uses a switch statement to map each key code (assumed
 * to be predefined constants like IDC_BUTTON_0, IDC_BUTTON_1, etc.)
 * to its corresponding integer value.
 *
 * For decimal digits (0-9), the function returns the digit's value.
 * For hexadecimal digits (A-F), it returns values 10-15 respectively.
 *
 * If an invalid or unsupported key code is provided, the function
 * returns -1 to indicate an error.
 *
 * @param keyCode   The key code of the pressed calculator button
 * @return          The corresponding digit value (0-15) or -1 for invalid input
 */
int convertKeyToDigit(DWORD keyCode)
{
    switch (keyCode)
    {
        case IDC_BUTTON_0:
            return 0;
        case IDC_BUTTON_1:
            return 1;
        case IDC_BUTTON_2:
            return 2;
        case IDC_BUTTON_3:
            return 3;
        case IDC_BUTTON_4:
            return 4;
        case IDC_BUTTON_5:
            return 5;
        case IDC_BUTTON_6:
            return 6;
        case IDC_BUTTON_7:
            return 7;
        case IDC_BUTTON_8:
            return 8;
        case IDC_BUTTON_9:
            return 9;
        case IDC_BUTTON_A:
            return 10;
        case IDC_BUTTON_B:
            return 11;
        case IDC_BUTTON_C:
            return 12;
        case IDC_BUTTON_D:
            return 13;
        case IDC_BUTTON_E:
            return 14;
        case IDC_BUTTON_F:
            return 15;
        default:
            return -1; // Invalid key
    }
}

/*
 * isClearKey(DWORD keyPressed)
 *
 * This function determines whether the pressed key is a clear key (CE or C).
 * It is used to identify when the user wants to clear the current entry or reset
 * the entire calculator state.
 *
 * @param keyPressed The ID of the button that was pressed
 * @return BOOL TRUE if the pressed key is a clear key, FALSE otherwise
 */
BOOL isClearKey(DWORD keyPressed)
{
    return (keyPressed == IDC_BUTTON_CE || keyPressed == IDC_BUTTON_C);
}

/*
 * isNumericInput(DWORD keyPressed)
 *
 * This function determines whether the pressed key represents a numeric input.
 * It is used to identify when the user is entering numbers into the calculator.
 *
 * The function performs the following tasks:
 * 1. Checks if the keyPressed value is within the range of numeric button IDs (0-9)
 * 2. Checks if the keyPressed value is a hexadecimal digit (A-F) for scientific mode
 * 3. Returns TRUE if the key is a valid numeric input, FALSE otherwise
 *
 * @param keyPressed The ID of the button that was pressed
 * @return BOOL TRUE if the pressed key is a numeric input, FALSE otherwise
 */
BOOL isNumericInput(DWORD keyPressed)
{
    // Check for digits 0-9
    if (keyPressed >= IDC_BUTTON_0 && keyPressed <= IDC_BUTTON_9) {
        return TRUE;
    }
    
    // Check for hexadecimal digits A-F (only in scientific mode)
    if (calcState.mode == SCIENTIFIC_MODE && 
        keyPressed >= IDC_BUTTON_A && keyPressed <= IDC_BUTTON_F) {
        return TRUE;
    }
    
    return FALSE;
}

/*
 * isOperatorKey(DWORD keyPressed)
 *
 * This function determines whether the pressed key represents an arithmetic operator.
 * It is used to identify when the user is entering an arithmetic operation.
 *
 * The function performs the following tasks:
 * 1. Checks if the keyPressed value matches any of the basic arithmetic operator button IDs
 * 2. Checks for additional operators in scientific mode
 * 3. Returns TRUE if the key is an operator, FALSE otherwise
 *
 * @param keyPressed The ID of the button that was pressed
 * @return BOOL TRUE if the pressed key is an operator, FALSE otherwise
 */
BOOL isOperatorKey(DWORD keyPressed)
{
    // Check for basic arithmetic operators
    switch (keyPressed)
    {
        case IDC_BUTTON_DIV:  // 0x8A
        case IDC_BUTTON_MUL:  // 0x8E
        case IDC_BUTTON_SUB:  // 0x92
        case IDC_BUTTON_ADD:  // 0x96
        case IDC_BUTTON_EQ:  // 0x97
            return TRUE;
    }

    // Check for additional operators
    switch (keyPressed)
    {
        case IDC_BUTTON_SQRT: // 0x98
        case IDC_BUTTON_PERC: // 0x99
        case IDC_BUTTON_INV:  // 0x9A
            return TRUE;

        // Operators available only in scientific mode
        case IDC_BUTTON_SIN:  // 0x9C
        case IDC_BUTTON_COS:  // 0x9D
        case IDC_BUTTON_TAN:  // 0x9E
        case IDC_BUTTON_ASIN: // 0x9F
        case IDC_BUTTON_ACOS: // 0xA0
        case IDC_BUTTON_ATAN: // 0xA1
        case IDC_BUTTON_LOG:  // 0xA2
        case IDC_BUTTON_LN:   // 0xA3
        case IDC_BUTTON_EXP:  // 0xA4
        case IDC_BUTTON_XY:   // 0xA5
        case IDC_BUTTON_SQR:  // 0xA9
        case IDC_BUTTON_CUBE: // 0xAA
        case IDC_BUTTON_FACT: // 0xAB
        case IDC_BUTTON_MOD:  // 0xAD
            return (calcState.mode == SCIENTIFIC_MODE);

        // Bitwise operators (assuming they're only in scientific mode)
        case IDC_BUTTON_AND:  // 0xB8
        case IDC_BUTTON_OR:   // 0xB9
        case IDC_BUTTON_XOR:  // 0xBA
        case IDC_BUTTON_NOT:  // 0xBB
        case IDC_BUTTON_LSH:  // 0xBC
            return (calcState.mode == SCIENTIFIC_MODE);

        default:
            return FALSE;
    }
}

BOOL isValueOverflow(int digit) {
    switch (calcState.mode) {
        case STANDARD_MODE:
            // 32-bit integer overflow check
            if (calcState.accumulatedValue > (LONG_MAX - digit) / (LONG)calcState.numberBase) {
                return true;
            }
            break;

        case SCIENTIFIC_MODE:
            // 64-bit integer overflow check 
            if (calcState.accumulatedValue > (LONGLONG_MAX - digit) / (LONGLONG)calcState.numberBase) {
                return true;
            }

            // Convert accumulatedValue string to a double for overflow check
            double accumulatedDouble = strtod(calcState.accumulatedValue, NULL);

            // Also check for floating-point overflow
            if (!isfinite(accumulatedDouble * calcState.numberBase + digit)) { 
                return true;
            }
            break;

        case SCIENTIFIC_NOTATION:
            // Convert the accumulated value (string) to ExtendedFloat80
            stringToExtendedFloat80(calcState.accumulatedValue);

            // Check for overflow using the updated function 
            if (isValueOverflowExtended()) {
                return true;
            }
            break;

        default:
            // Unknown mode, assume overflow for safety
            return true;
    }

    return false;
}

/*
 * isPreviousKeyOperator()
 *
 * This function determines whether the previously pressed key was an operator
 * in the calculator application. It checks for various types of operator keys
 * and considers the current calculator mode.
 *
 * The function performs the following checks:
 * 1. Checks if the key is within the range of basic operator keys
 * 2. Checks for additional operator keys that are always considered operators
 * 3. In scientific mode, checks for scientific operator keys
 *
 * Operator keys include:
 * - Basic arithmetic operators (add, subtract, multiply, divide)
 * - Special operators like square root, percentage, and inverse
 * - In scientific mode: trigonometric functions, logarithms, etc.
 *
 * This function is crucial for proper input handling and determining how to
 * process subsequent key presses based on whether the previous key was an operator.
 *
 * @param state    Pointer to the current calculator state
 * @return         TRUE if the previous key was an operator, FALSE otherwise
 */
BOOL isPreviousKeyOperator()
{

    // Define the range of operator key codes
    const int MIN_OPERATOR_KEY = 0x56;  // Assuming 0x56 is the lowest operator key code
    const int MAX_OPERATOR_KEY = 0x5F;  // Assuming 0x5F is the highest operator key code

    // Check if the key is within the basic operator key range
 // Check if the previous key pressed falls within the operator range
    if (calcState.keyPressed >= MIN_OPERATOR_KEY && calcState.keyPressed <= MAX_OPERATOR_KEY) {
        return TRUE;
    }

    // Check for additional operators
    switch (calcState.keyPressed)
    {
        case IDC_BUTTON_SQRT:
        case IDC_BUTTON_PERC:
        case IDC_BUTTON_INV:
            return TRUE;

        // Operators available only in scientific mode
        case IDC_BUTTON_SIN:
        case IDC_BUTTON_COS:
        case IDC_BUTTON_TAN:
        case IDC_BUTTON_ASIN:
        case IDC_BUTTON_ACOS:
        case IDC_BUTTON_ATAN:
        case IDC_BUTTON_LOG:
        case IDC_BUTTON_LN:
        case IDC_BUTTON_EXP:
        case IDC_BUTTON_AND:
        case IDC_BUTTON_OR:
        case IDC_BUTTON_XOR:
        case IDC_BUTTON_NOT:
        case IDC_BUTTON_LSH:
            return (calcState.mode == SCIENTIFIC_MODE);

        default:
            return FALSE;
    }
}

/*
 * isSpecialFunctionKey()
 *
 * This function determines whether a given key press represents a special function
 * in the calculator application. It checks for both standard special keys and
 * mode-specific special keys.
 *
 * The function performs the following checks:
 * 1. Checks if the key is within a predefined range of special function keys
 * 2. Checks for specific individual keys that are always considered special
 * 3. In scientific mode, checks for additional keys that become special functions
 *
 * Special keys include:
 * - Toggle scientific mode (0x54)
 * - Memory-related functions (0x6d, 0x70)
 * - Backspace or clear entry (0x53)
 * - A range of keys from 0x7d to 0x81
 * - In scientific mode, additional keys from 0x74 to 0x78
 *
 * This function is crucial for proper input handling and determining when to
 * trigger special calculator functions instead of standard numeric input.
 *
 * @param keyPressed   The key code of the pressed key
 * @return             true if the key is a special function key, false otherwise
 */
BOOL isSpecialFunctionKey(DWORD keyPressed) {
    // Define special function key ranges and individual keys
    const DWORD SPECIAL_KEY_START = 0x7d;
    const DWORD SPECIAL_KEY_END = 0x81;

    // Check if the key is within the special function key range
    if (keyPressed >= SPECIAL_KEY_START && keyPressed <= SPECIAL_KEY_END) {
        return true;
    }

    // Check for individual special function keys
    switch (keyPressed) {
    case 0x54:  // Likely the toggle scientific mode key
    case 0x6d:  // Possibly memory-related function
    case 0x70:  // Possibly another memory-related function
    case 0x53:  // Possibly backspace or clear entry
        return true;
    }

    // Check for keys that might be special based on current mode
    if (calcState.mode == SCIENTIFIC_MODE) {
        // Additional keys that are special in scientific mode
        if (keyPressed >= 0x74 && keyPressed <= 0x78) {
            return true;
        }
    }

    // If none of the above conditions are met, it's not a special function key
    return false;
}


/*
 * updateInputMode()
 *
 * This function manages the calculator's input mode based on the key pressed.
 * It determines whether to activate or deactivate the input mode, which affects
 * how subsequent key presses are interpreted.
 * Key codes and their functions:
 * // 0x55: Decimal point
 * // 0x29: Right parenthesis or equals sign
 * // 0x56-0x73: Various operation keys
 * // 0x75-0x7c: Function keys
 * // 0x12d: Unknown function, possibly clear or cancel
 *
 * Input mode activation allows the calculator to start a new calculation or continue
 * the current one, while deactivation typically occurs when an operation is completed
 * or a function is applied. This function works in conjunction with other mode-related
 * functions like toggleScientificMode and updateToggleButton to maintain the calculator's state.
 *
 * @param keyPressed   The key code of the pressed key
 * @return             TRUE if the input mode was changed, FALSE otherwise
 */
BOOL updateInputMode(DWORD keyPressed) {
    if (!calcState.isInputModeActive) {
        // Check if the key is a valid input to activate input mode
        if (isNumericInput(keyPressed) || keyPressed == 0x55) {
            calcState.isInputModeActive = TRUE;
            InitcalcState(&calcState);
            return TRUE;
        }
    }
    else {
        // Check if the key should deactivate input mode
        if (isOperatorKey(keyPressed) || keyPressed == 0x29 ||
            (keyPressed >= 0x56 && keyPressed < 0x74) ||
            (keyPressed > 0x74 && keyPressed < 0x7d) ||
            keyPressed == 0x12d) {
            calcState.isInputModeActive = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}