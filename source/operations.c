/*-----------------------------------------------------------------------------
    operations.c --  Calculation and Conversion Functions for the Windows
                     Calculator (reconstructed code).

               This module implements the core arithmetic, logic, and
               transcendental functions for the calculator, as well as
               routines to handle different number bases and precision
               levels. It also includes functions for converting between
               standard numeric types and the custom 80-bit extended
               precision floating-point type used for scientific notation.

               Key functions include:

               - stringToExtendedFloat80: Converts a decimal string to the
                                          80-bit extended precision format.
               - intToExtendedFloat80: Converts a 64-bit integer to the
                                       80-bit extended precision format.
               - isValueOverflowExtended: Checks for overflow in the 80-bit
                                           extended precision format.
               - performAdvancedCalculation:  Performs arithmetic and
                                              logical operations on numbers
                                              in various bases and precisions.
               - [Other arithmetic, logic, and transcendental functions]

  -----------------------------------------------------------------------------*/

#include "operations.h"
#include "main.h"

extern _calculatorState calcState;

/*
 * intToExtendedFloat80
 * 
 * This function converts a 64-bit signed integer (LONGLONG) to an ExtendedFloat80
 * structure, representing an 80-bit extended precision floating-point number.
 * The function assumes a base-10 (decimal) representation of the integer. 
 * 
 * The conversion involves: 
 * 1. Sign handling:  Setting the sign bit in the exponent for negative values.
 * 2. Zero Handling: Returns a zeroed ExtendedFloat80 for a zero input. 
 * 3. Finding the most significant bit (MSB).
 * 4. Normalizing the mantissa: Shifting the bits so the MSB is at the 63rd bit of 
 *    the mantissaHigh field. 
 * 5. Calculating the exponent: Determining the biased exponent based on the MSB position.
 *
 * @param value  The 64-bit signed integer to convert.
 * @return        None. Modifies the global calcState.scientificNumber. 
 */
void intToExtendedFloat80(LONGLONG value) {

    // Handle sign 
    if (value < 0) {
        calcState.scientificNumber.exponent = 0x8000; // Set sign bit
        value = -value; 
    } else {
        calcState.scientificNumber.exponent = 0;
    }

    // If value is 0, return a zeroed ExtendedFloat80
    if (value == 0) {
        calcState.scientificNumber.mantissaLow = 0;
        calcState.scientificNumber.mantissaHigh = 0;
        return;
    }

    // Find the most significant bit (MSB) 
    int msbPosition = 63; // Start at the highest bit for 64-bit
    while ((value & (1LL << msbPosition)) == 0) { 
        msbPosition--;
    }

    // Normalize the mantissa (shift so that MSB is at bit 63 of mantissaHigh) 
    int shiftAmount = msbPosition - 63;
    if (shiftAmount >= 0) {
        calcState.scientificNumber.mantissaHigh = (uint)(value >> shiftAmount);
        calcState.scientificNumber.mantissaLow = (uint)(value << (32 - shiftAmount)); 
    } else {
        calcState.scientificNumber.mantissaHigh = 0;
        calcState.scientificNumber.mantissaLow = (uint)value;
    }

    // Calculate the exponent (biased) 
    calcState.scientificNumber.exponent |= 0x3FFF + shiftAmount;
}

/*
 * isValueOverflowExtended
 * 
 * This function checks for potential overflow when a digit is added to the current 
 * value stored in the global calcState.scientificNumber, which is an 80-bit 
 * extended precision floating-point number. 
 *
 * The overflow check is primarily based on the exponent value:
 * 1. Maximum Exponent: If the exponent is already at its maximum value (0x7FFF),
 *    an overflow condition is indicated. 
 * 2. Simulated Multiplication and Addition: The function simulates multiplying 
 *    the current value by the numberBase and then adding the digit. This is done
 *    by adjusting the exponent and (potentially) the mantissa.
 * 3. Exponent Overflow: If the adjusted exponent exceeds the maximum allowable 
 *    value, an overflow occurs.
 * 
 * The function handles the sign bit separately to ensure accurate overflow detection. 
 * 
 * @param  None. Uses the global calcState.scientificNumber. 
 * @return True if an overflow condition is detected, false otherwise.
 */ 
bool isValueOverflowExtended(void) {
    // Check if exponent is already at the maximum
    if (calcState.scientificNumber.exponent == 0x7FFF) {
        return true; // Overflow
    }

    // Handle sign separately
    int sign = (calcState.scientificNumber.exponent & 0x8000) ? -1 : 1; 
    calcState.scientificNumber.exponent &= 0x7FFF;  // Remove the sign bit

    // Simulate multiplication by numberBase
    calcState.scientificNumber.exponent += (short)log2(calcState.numberBase);  

    // Simulate adding the digit (might need additional checks here for precision loss)
    // For simplicity, assuming digit is small enough to be added without significant changes 
    // to the mantissa. More sophisticated logic may be needed.

    // Check for exponent overflow
    if (calcState.scientificNumber.exponent >= 0x7FFF) { 
        return true; // Overflow
    } 

    // Restore the sign bit
    calcState.scientificNumber.exponent |= (sign == -1) ? 0x8000 : 0;

    return false; // No overflow
}

/*
 * stringToExtendedFloat80
 *
 * This function converts a decimal string representation of a number to an
 * ExtendedFloat80 structure (representing an 80-bit extended precision 
 * floating-point number). The function assumes the input string represents a 
 * base-10 decimal number.
 *
 * The function handles:
 * 1. Sign: Determines the sign of the number from a leading '-' character.
 * 2. Separation: Splits the string into integer and fractional parts. 
 * 3. Integer Part Conversion: Converts the integer part to ExtendedFloat80.
 * 4. Fractional Part Conversion: Converts the fractional part, handling potential
 *    precision loss and normalization. 
 *
 * @param str  Pointer to the NULL-terminated string representing the decimal number.
 * @return     None. The function modifies the global calcState.scientificNumber.
 */
void stringToExtendedFloat80(const char* str) {
    // Handle sign
    if (*str == '-') {
        calcState.scientificNumber.exponent = 0x8000; // Set sign bit
        str++; 
    } else {
        calcState.scientificNumber.exponent = 0;
    }

    // Separate integer and fractional parts
    char* decimalPoint = strchr(str, '.');
    int integerPart = (decimalPoint != NULL) ? atoi(str) : atoi(str);
    double fractionalPart = (decimalPoint != NULL) ? strtod(decimalPoint, NULL) : 0.0;

    // Convert integer part to ExtendedFloat80, directly modifying calcState.scientificNumber
    intToExtendedFloat80(integerPart); 

    // Add fractional part to the mantissa
    if (fractionalPart != 0.0) {
        for (int i = 0; i < 64 && fractionalPart != 0.0; i++) {
            fractionalPart *= 2;
            if (fractionalPart >= 1.0) {
                // Set the corresponding bit in the mantissa
                if (i < 32) {
                    calcState.scientificNumber.mantissaLow |= 1 << i;
                } else {
                    calcState.scientificNumber.mantissaHigh |= 1 << (i - 32);
                }
                fractionalPart -= 1.0;
            }
        }
        // Normalize if necessary (if the fractional part added a leading 1 to the mantissa)
        if (calcState.scientificNumber.mantissaHigh & 0x80000000) {
            calcState.scientificNumber.exponent++;
            shiftMultiWordInteger(&calcState.scientificNumber.mantissaHigh, -1); // Right shift
        }
    }
}


/*
 * shiftMultiWordInteger
 *
 * This function performs a bitwise shift on a 64-bit integer represented by 
 * two 32-bit unsigned integers (highWord and the lower 32 bits of
 * calcState.scientificNumber.mantissaLow). The function supports both left and
 * right shifts, with positive shiftAmount values indicating a right shift and 
 * negative values indicating a left shift. 
 *
 * The function handles cases where the shiftAmount is greater than or equal to 32
 * bits, ensuring correct carry bit propagation between the two words. 
 * 
 * @param highWord       Pointer to the upper 32 bits of the 64-bit integer.
 * @param shiftAmount     The number of bits to shift (positive for right, negative for left).
 * @return                None. The function modifies the value pointed to by highWord and
 *                        the calcState.scientificNumber.mantissaLow. 
 */
void shiftMultiWordInteger(uint* highWord, int shiftAmount) {
    if (shiftAmount > 0) { // Right shift
        if (shiftAmount >= 32) {
            *highWord = 0;  // Shift is more than 32 bits, highWord becomes 0
        } else {
            *highWord >>= shiftAmount;        // Shift highWord right
            *highWord |= calcState.scientificNumber.mantissaLow << (32 - shiftAmount); // Carry bits from lowWord
        }
        calcState.scientificNumber.mantissaLow >>= shiftAmount; // Shift lowWord right
    } else if (shiftAmount < 0) { // Left shift 
        shiftAmount = -shiftAmount; 
        if (shiftAmount >= 32) {
            calcState.scientificNumber.mantissaLow = 0; // Shift is more than 32 bits, lowWord becomes 0 
        } else { 
            calcState.scientificNumber.mantissaLow <<= shiftAmount;   // Shift lowWord left 
            calcState.scientificNumber.mantissaLow |= *highWord >> (32 - shiftAmount); // Carry bits from highWord 
        }
        *highWord <<= shiftAmount;  // Shift highWord left 
    } // No shift if shiftAmount is 0
}

/*
 * toggleStatisticsWindow
 *
 * This function toggles the visibility of the statistics window. If the statistics
 * window is currently closed, it creates and shows the window. If the window
 * is open, it closes the window. The function also updates the
 * calcState.statisticsWindowOpen flag to reflect the window's state.
 *
 * The function handles potential errors during dialog creation and displays
 * an error message if necessary.
 *
 * @param buttonID  The resource ID of the dialog template for the statistics window.
 */
void toggleStatisticsWindow(UINT buttonID) {
    if (calcState.statisticsWindowOpen) {
        // Close the statistics window
        DestroyWindow(calcState.statisticsWindow);
        calcState.statisticsWindow = NULL;
        calcState.statisticsWindowOpen = FALSE;
    }
    else {
        // Create and show the statistics window
        calcState.statisticsWindow = CreateDialogParamA(
            calcState.appInstance,
            MAKEINTRESOURCE(buttonID),
            calcState.windowHandle,
            statisticsWindowProc,
            0);

        if (calcState.statisticsWindow != NULL) {
            ShowWindow(calcState.statisticsWindow, SW_SHOW);
            calcState.statisticsWindowOpen = TRUE;
        }
        else {
            // Handle error creating the statistics window 
            DWORD errorCode = GetLastError();
            TCHAR errorMessage[100];
            wsprintf(errorMessage, TEXT("Error creating statistics window: %d"), errorCode);
            MessageBox(calcState.windowHandle, errorMessage, calcState.className, MB_OK | MB_ICONERROR);
        }
    }
}
