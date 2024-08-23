/*
 * isSpecialFunctionKey
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
bool isSpecialFunctionKey(DWORD keyPressed) {
    // Define special function key ranges and individual keys
    const uint SPECIAL_KEY_START = 0x7d;
    const uint SPECIAL_KEY_END = 0x81;

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
    if (calculatorMode == SCIENTIFIC_MODE) {
        // Additional keys that are special in scientific mode
        if (keyPressed >= 0x74 && keyPressed <= 0x78) {
            return true;
        }
    }

    // If none of the above conditions are met, it's not a special function key
    return false;
}


/*
 * updateInputMode
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