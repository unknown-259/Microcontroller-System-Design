/* Template ver 1.0 W2022
 * File:   uart.c
 * Author: Nathan Tran
 * Brief: 
 * Created on January 28, 2025, 2:48 pm
 * Modified on <month> <day>, <year>, <hour> <pm/am>
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/
#include <stdio.h>        // For testing buffer
#include "uart.h"         // UART header file with function prototypes
#include "BOARD.h"        // Board-specific functions
#include <xc.h>           // PIC32-specific functions
#include "sys/attribs.h"  // System interrupt attributes
#include <string.h>       // For string manipulation functions like strlen()

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define BAUD 115200         // Define baud rate for UART communication
#define BuffSize 256        // Define buffer size for UART data

/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/
typedef enum {
    FULL, EMPTY, FULL_NOT, EMPTY_ERROR
} fullness;  // Enum to track buffer states: FULL, EMPTY, or any errors

/*******************************************************************************
 * GLOBAL DATA                                                                 *
 ******************************************************************************/
static buff RXbuff;  // RX buffer to store received data
static buff TXbuff;  // TX buffer to store data to be transmitted
static int putTrans; // Flag to indicate if PutChar() is running
static int TXcollision; // Flag to indicate transmission space is available while PutChar() is running

/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                 *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS                                             *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS                                            *
 ******************************************************************************/

/*******************************************************************************
 * Private Functions to Add to Your Uart.c File                                 
 ******************************************************************************/

// Initialize UART with specified baud rate
void Uart_Init(unsigned long baudRate) {
    // Initialize RX and TX buffers
    buff_init(&TXbuff);
    buff_init(&RXbuff);
    
    U1MODE = 0x00;  // Clear UART settings for 8-bit Data, no Parity, and 1 Stop bit
    U1STA = 0x00;   // Clear Transmitter and Receiver status
    U1MODEbits.ON = 1;  // Enable UART module

    // Set baud rate based on system clock and desired baud rate
    U1BRG = ((40000000) / (16 * baudRate)) - 1;  // Calculate baud rate setting
    
    U1MODEbits.PDSEL = 0;  // Set parity to none
    U1MODEbits.STSEL = 0;  // Set stop bits to 1
    U1STAbits.URXEN = 1;   // Enable UART receiver
    U1STAbits.UTXEN = 1;   // Enable UART transmitter
    U1STAbits.UTXISEL = 0b00; // Interrupt when TX buffer is empty
    U1STAbits.URXISEL = 0b00; // Interrupt when RX has >= 1 byte
    
    // Enable UART interrupt for both RX and TX
    IEC0bits.U1RXIE = 1;  // Enable RX interrupt
    IEC0bits.U1TXIE = 1;  // Enable TX interrupt
    
    IPC6bits.U1IP = 4;  // Set UART interrupt priority to 4
    IPC6bits.U1IS = 0;  // Set UART interrupt sub-priority to 0
    
    // Clear interrupt flags
    IFS0bits.U1RXIF = 0;
    IFS0bits.U1TXIF = 0;
}

// Adds a character to the TX buffer
int PutChar(char ch) {
    putTrans = TRUE;  // Set flag indicating PutChar is running
    
    if (buffState(&TXbuff) == FULL) {  // Check if TX buffer is full
        return ERROR;  // Return ERROR if TX buffer is full
    }
    
    if (buff_add(&TXbuff, ch) == SUCCESS) {  // Add character to TX buffer
        putTrans = FALSE;
        TXcollision = FALSE;
    } else {
        putTrans = FALSE;
        return ERROR;  // Return ERROR if buffer addition fails
    }
    
    if (U1STAbits.TRMT) {  // Check if the transmit shift register is empty
        IFS0bits.U1TXIF = 1;  // Set the TX interrupt flag to trigger transmission
    }
    
    if (TXcollision == TRUE) {  // Handle TX collision
        TXcollision = FALSE;
        IFS0bits.U1TXIF = 1;  // Trigger TX interrupt again
    }
    
    return SUCCESS;
}

// Retrieves a character from the RX buffer
int GetChar(unsigned char *ch) {
    return buff_read(&RXbuff, ch);  // Read from RX buffer
}

/**
 * Custom version of the putc function to redirect output to UART1.
 * Refer to ...\docs\MPLAB C32 Libraries.pdf: 32-Bit Language Tools Library.
 * Section 2.13.2 discusses _mon_putc() and how it uses UART2 by default.
 * This function can redirect output to UART1.
 */
void _mon_putc(char c) {
    // Code to redirect the character 'c' to UART1 transmission
}

// Print a string using UART (untested/unimplemented)
int printU(const char *format) {
    char test[128];
    sprintf(test, format);  // Format the string
    unsigned char testLen = strlen(test);  // Get string length
    
    for (int i = 0; i < testLen - 1; i++) {  // Loop through each character in the string
        if (PutChar(test[i]) == ERROR) {
            return ERROR;  // Return ERROR if PutChar fails
        }
    }
    return SUCCESS;
}

/****************************************************************************
 * Function: IntUart1Handler
 * Parameters: None.
 * Returns: None.
 * Description: Interrupt Service Routine (ISR) for UART1. This function handles both
 *              transmit and receive interrupts. It processes incoming data and manages
 *              outgoing data transmission.
 ****************************************************************************/

// Initializes buffers for RX and TX communication
void buff_init(buff *buffinit) {
    buffinit->head = 0;   // Initialize head of buffer
    buffinit->tail = 0;   // Initialize tail of buffer
}

// Returns the current state of the buffer: FULL, EMPTY, or FULL_NOT
int buffState(buff *buffer3) {
    if ((((buffer3->head) + 1) % BuffSize) == buffer3->tail) {  // Check if buffer is full
        return FULL;
    } else if ((buffer3->tail) == (buffer3->head)) {  // Check if buffer is empty
        return EMPTY;
    }
    return FULL_NOT;  // Buffer is neither full nor empty
}

// Adds an item to the buffer if space is available
int buff_add(buff *buffer1, uint8_t added) {
    if (buffState(buffer1) != FULL) {  // Ensure buffer is not full
        buffer1->buffArray[buffer1->head] = added; // Add item to buffer at the head
        buffer1->head = ((buffer1->head) + 1) % BuffSize;  // Increment head with wraparound
        return SUCCESS;
    } else {
        return FULL;  // Return FULL if no space is available
    }
}

// Reads and removes an item from the buffer from the tail
int buff_read(buff *buffer1, char *ch) {
    if (buffState(buffer1) != EMPTY) {  // Ensure buffer is not empty
        *ch = buffer1->buffArray[buffer1->tail]; // Retrieve the item at the tail
        buffer1->tail = ((buffer1->tail) + 1) % BuffSize;  // Increment tail with wraparound
        return SUCCESS;
    } else {
        return ERROR;  // Return ERROR if buffer is empty
    }
}

/**
 * UART1 Interrupt Service Routine (ISR) for handling TX and RX interrupts.
 * This function handles the RX interrupt to read data and the TX interrupt
 * to send data from the TX buffer.
 */
void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
    // Handle RX interrupt if RX flag is set
    if (IFS0bits.U1RXIF) {
        IFS0bits.U1RXIF = 0;  // Clear RX interrupt flag
        
        // Read characters from the RX buffer and add to the RX buffer
        while (U1STAbits.URXDA == 1) {
            buff_add(&RXbuff, U1RXREG);  // Add received byte to RX buffer
        }

        // Check if there was a receive buffer overrun (OERR)
        if (U1STAbits.OERR == 1) {
            U1STAbits.OERR = 0;  // Clear the overrun error
        }
    }

    // Handle TX interrupt if TX flag is set
    if (IFS0bits.U1TXIF) {
        IFS0bits.U1TXIF = 0;  // Clear TX interrupt flag
        
        if (putTrans == TRUE) {
            TXcollision = TRUE;  // Set TX collision flag
        } else {
            unsigned char foo;
            while (!U1STAbits.UTXBF && buffState(&TXbuff) != EMPTY) {  // Check if TX buffer is not full
                buff_read(&TXbuff, &foo);  // Read data from TX buffer
                U1TXREG = foo;  // Send data via UART
            }
        }
    }
}
