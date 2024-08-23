// Function to determine if a key press represents a special function
// Special functions include mode toggles, memory operations, and clear/backspace
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


// Function to check and update the calculator's input mode
// Key hex values:
// 0x55: Decimal point
// 0x29: Right parenthesis or equals sign
// 0x56-0x73: Various operation keys
// 0x75-0x7c: Function keys
// 0x12d: Unknown function, possibly clear or cancel
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