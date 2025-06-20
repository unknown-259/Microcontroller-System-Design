/* Template version 1.0 W2022
 * File:   FreeRunningTimer.c
 * Author: Nathan Tran
 * Brief: Implements a free-running timer using Timer 5 for time measurement in milliseconds and microseconds.
 * Created on: <month> <day>, <year>
 * Modified on: <month> <day>, <year>, <hour> <pm/am>
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/
#include "BOARD.h"
#include "FreeRunningTimer.h" // The header file for this source file. 
#include <sys/attribs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Protocol2.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define SYSTEM_CLOCK 80000000L // System clock frequency
#define PB_CLOCK SYSTEM_CLOCK/2 // Peripheral bus clock
#define BAUDRATE 115200 // UART baud rate for communication
#define test

/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/

/*******************************************************************************
 * GLOBAL VARIABLES                                                            *
 ******************************************************************************/
static unsigned int millitime; // Global variable to store millisecond count
static unsigned int microtime; // Global variable to store microsecond count

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS                                             *
 ******************************************************************************/

/**
 * @brief Initializes Timer 5 for time measurement, sets up the necessary registers and configurations.
 */
void FreeRunningTimer_Init(void) {
    BOARD_Init();
    T5CONbits.ON = 0; // Clear Timer 5 control register
    T5CONbits.TCKPS = 0b011; // Set prescaler to 1:8
    TMR5 = 0x0; // Reset Timer 5 counter
    PR5 = 0x1387; // Set period match value for Timer 5
    IFS0bits.T5IF = 0; // Clear Timer 5 interrupt flag
    IPC5bits.T5IP = 0b011; // Set interrupt priority to 3
    IPC5bits.T5IS = 0b0; // Set interrupt subpriority to 0
    IEC0bits.T5IE = 1; // Enable Timer 5 interrupt
    T5CONbits.ON = 1; // Start Timer 5
}

/**
 * @brief Returns the elapsed time in milliseconds since the timer was initialized.
 * 
 * @return unsigned int The current millisecond count.
 */
unsigned int FreeRunningTimer_GetMilliSeconds(void){     
    return millitime;
}

/**
 * @brief Returns the elapsed time in microseconds based on the timer and millisecond counter.
 * 
 * @return unsigned int The current microsecond count.
 */
unsigned int FreeRunningTimer_GetMicroSeconds(void) {
    microtime = millitime + (TMR5/5); // Calculate microseconds based on the current Timer 5 value
    return microtime;
}

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS                                            *
 ******************************************************************************/

/**
 * @brief Interrupt Service Routine (ISR) for Timer 5.
 * Increments the millisecond counter and clears the interrupt flag.
 */
void __ISR(_TIMER_5_VECTOR) Timer5IntHandler(void) {
    if (IFS0bits.T5IF == 1) {
        millitime++; // Increment millisecond counter
        IFS0bits.T5IF = 0; // Clear Timer 5 interrupt flag
    }
}

#ifdef test
/**
 * @brief Main test function to demonstrate the functionality of the free-running timer.
 * Toggles LEDs and sends debug messages at regular intervals.
 */
int main() {
    int x = 0;
    FreeRunningTimer_Init(); // Initialize the free-running timer
    Protocol_Init(BAUDRATE); // Initialize UART communication for debug messages
    char message[128];
    unsigned int lastTime = 0;
    int LEDswap = 0;
    int timeCount = 2;
    sprintf(message, "Nathan Tran's FreeRunningTimer");
    Protocol_SendDebugMessage(message); // Send an initial debug message
    while (1) {
        if ((FreeRunningTimer_GetMilliSeconds() - lastTime) >= 2000) { // Every 2 seconds
            char milliarray[128];
            sprintf(milliarray, "Time: %i milliseconds, %i microseconds", FreeRunningTimer_GetMilliSeconds(), FreeRunningTimer_GetMicroSeconds());
            Protocol_SendDebugMessage(milliarray); // Send time debug message
            timeCount += 2;
            lastTime = FreeRunningTimer_GetMilliSeconds();
            // Toggle LEDs each time the condition is met
            if (LEDswap == 1) {
                LEDS_SET(0x00);
                LEDswap--;
            } else if (LEDswap == 0) {
                LEDS_SET(0b00000001);
                LEDswap++;
            }
        }
        runProtocol(); // Execute protocol functions
    }
    return (EXIT_SUCCESS);
}
#endif
on of the function.

  @Remarks
    Refer to the example_file.h interface header for function usage details.
 */
int ExampleInterfaceFunction(int param1, int param2) {
    return 0;
}


/* *****************************************************************************
 End of File
 */
