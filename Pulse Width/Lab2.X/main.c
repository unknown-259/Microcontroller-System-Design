/* Template ver 1.0 W2022
 * File: main.c
 * Author: Nathan Tran
 * Brief:
 * Created on February 19, 2025, 2:00 pm
 * Modified on <month> <day>, <year>, <hour> <pm/am
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "MessageIDs.h"
#include "uart.h"
#include "Protocol2.h"
#include "BOARD.h"

// Define constants for baud rate and testing protocol
#define baudRate 115200  // Define baud rate for UART communication
#define pt2

int main() {
    // Initialize the hardware board and necessary components
    BOARD_Init();
    
    // Initialize UART communication with the specified baud rate
    Uart_Init(BAUD);

    // FOR PT 1
#ifdef pt1
    // Start an infinite loop for PT1 where we simply echo any received data
    while (1) {
        // Check if data has been received on UART1
        if (U1STAbits.URXDA) {
            // Create a variable to hold the received data
            int x = 0;
            
            // Echo the received byte back to UART1
            U1TXREG = U1RXREG;  // Copy received data to transmit register to send it back
        }
    }
#endif
    
    // FOR PT 2
#ifdef pt2
    // Declare variables to hold received data for PT2
    unsigned char foo2;
    unsigned char foo3;
    
    // Start an infinite loop for PT2 where data is received and then echoed back
    while(1) {
        // Attempt to receive a character using GetChar function and check if it's successful
        if (GetChar(&foo2) == SUCCESS) {
            // If successful, send the received character back using PutChar
            PutChar(foo2);
        }
    }
#endif

    // FOR PT 3
#ifdef pt3
    // Initialize the protocol with the specified baud rate for PT3 functionality
    Protocol_Init(baudRate);
    
    // Start an infinite loop for PT3 where the protocol handling function is called
    while(1) {
        // Continuously run the protocol process
        runProtocol();  // This is where the protocol-specific tasks happen
    }
#endif

    // Return success status (though this point is never reached in an infinite loop)
    return (EXIT_SUCCESS);
}
