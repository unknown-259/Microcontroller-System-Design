/* Template ver 1.0 W2022
 * File: RCServo.c
 * Author: Nathan Tran
 * Brief: 
 * Created on 
 * Modified on <month> <day>, <year>, <hour> <pm/am
 */

#include "BOARD.h" // Board-specific definitions
#include "FreeRunningTimer.h" // Free-running timer functions
#include "uart.h" // UART communication functions
#include "Protocol2.h" // Protocol functions for communication
#include "RCServo.h" // Header file for RC servo functions
#include "MessageIDs.h" // Message ID definitions

#define TEST3 // Define for testing purposes
#define baudrate 115200 // Baud rate for UART communication
unsigned int pulse_width; // Variable to store the current pulse width for the servo

/**
 * @Function RCServo_Init
 * @brief Initializes Timer3 and Output Compare module 3 for RC servo control.
 * @return 1 on success
 * @note Configures Timer3 for a 50ms period and sets up OC3 for PWM signal generation.
 */
int RCServo_Init(void) {
    T3CON = 0x0; // Reset Timer3 configuration
    OC3CON = 0; // Reset Output Compare module 3
    TMR3 = 0x0000; // Clear Timer3 counter
    PR3 = 62500 - 1; // Set Timer3 period for 50ms (1:32 prescaler, 40 MHz clock)

    T3CONbits.TCKPS = 0b101; // Set Timer3 prescaler to 1:32
    T3CONbits.TON = 1; // Enable Timer3
    OC3CONbits.OCM = 0b101; // Set Output Compare mode to PWM mode without fault protection
    OC3R = 0; // Initialize OC3R (start time of pulse)
    OC3RS = 0; // Initialize OC3RS (end time of pulse, default to 600us)
    OC3CONbits.OCTSEL = 1; // Use Timer3 as the clock source for OC3

    IEC0bits.OC3IE = 0; // Disable OC3 interrupt initially
    IPC3bits.OC3IP = 6; // Set OC3 interrupt priority to 6
    IPC3bits.OC3IS = 3; // Set OC3 interrupt sub-priority to 3

    pulse_width = RC_SERVO_CENTER_PULSE; // Initialize pulse width to the center position

    // Enable Timer3, OC3, and interrupts
    IEC0bits.OC3IE = 1; // Enable OC3 interrupt
    IPC3bits.OC3IP = 6; // Set OC3 interrupt priority to 6
    IPC3bits.OC3IS = 3; // Set OC3 interrupt sub-priority to 3
    IFS0bits.OC3IF = 0; // Clear OC3 interrupt flag
    OC3CONbits.ON = 1; // Enable Output Compare module 3
    return 1; // Return success
}

/**
 * @Function RCServo_SetPulse
 * @param inPulse - Desired pulse width in timer ticks
 * @return 1 on success
 * @brief Sets the pulse width for the RC servo, ensuring it stays within valid limits.
 * @note Clamps the pulse width to the minimum and maximum allowed values.
 */
int RCServo_SetPulse(unsigned int inPulse) {
    if (inPulse < RC_SERVO_MIN_PULSE) {
        pulse_width = RC_SERVO_MIN_PULSE; // Clamp to minimum pulse width
    } else if (inPulse > RC_SERVO_MAX_PULSE) {
        pulse_width = RC_SERVO_MAX_PULSE; // Clamp to maximum pulse width
    } else {
        pulse_width = inPulse; // Use the provided pulse width
    }
    OC3RS = pulse_width - 1; // Update OC3RS with the new pulse width
    return 1; // Return success
}

/**
 * @Function RCServo_GetPulse
 * @return Current pulse width in timer ticks
 * @brief Returns the current pulse width being used for the RC servo.
 */
unsigned int RCServo_GetPulse(void) {
    return pulse_width; // Return the current pulse width
}

/**
 * @Function RCServo_GetRawTicks
 * @return Current value of OC3RS (pulse width in timer ticks)
 * @brief Returns the raw value of OC3RS, which determines the pulse width.
 */
unsigned int RCServo_GetRawTicks(void) {
    return OC3RS; // Return the current OC3RS value
}

/**
 * @Function __OC3Interrupt
 * @brief Interrupt Service Routine for Output Compare module 3.
 * @note Updates OC3RS with the current pulse width and clears the interrupt flag.
 */
void __ISR(_OUTPUT_COMPARE_3_VECTOR) __OC3Interrupt(void) {
    if (IEC0bits.OC3IE) { // Check if OC3 interrupt is enabled
        OC3RS = pulse_width; // Update OC3RS with the current pulse width
    }
    IFS0bits.OC3IF = 0; // Clear the OC3 interrupt flag
}