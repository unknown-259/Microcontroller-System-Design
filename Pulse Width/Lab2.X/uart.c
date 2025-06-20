/* Template ver 1.0 W2022
 * File:   uart.c
 * Author: Nathan Tran
 * Brief: 
 * Created on
 * Modified on 
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/
#include <stdio.h> // to test buffer
#include "uart.h" // The header file for this source file. 
#include "BOARD.h"
#include <xc.h>
#include "sys/attribs.h"
#include <string.h> // to use strlen())
/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define BAUD 115200
#define BuffSize 256

/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/

typedef enum {
    FULL, EMPTY, FULL_NOT, EMPTY_ERROR
} fullness;
/*******************************************************************************
 * GLOBAL DATA                                    
 *
 ******************************************************************************/

static buff RXbuff;
static buff TXbuff;
static int putTrans; // while PutChar() is running
static int TXcollision; // space is open for transmission while PutChar() is running

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
 * PRIVATE FUNCTIONS to add to your Uart.c file                                    
 *
 ******************************************************************************/

/**
 * Refer to ...\docs\MPLAB C32 Libraries.pdf: 32-Bit Language Tools Library.
 * In sec. 2.13.2 helper function _mon_putc() is noted as normally using
 * UART2 for STDOUT character data flow. Adding a custom version of your own
 * can redirect this to UART1 by calling your putchar() function.   
 */
void _mon_putc(char c) {
    //your code goes here
}

/****************************************************************************
 * Function: IntUart1Handler
 * Parameters: None.
 * Returns: None.
 * The PIC32 architecture calls a single interrupt vector for both the 
 * TX and RX state machines. Each IRQ is persistent and can only be cleared
 * after "removing the condition that caused it". This function is declared in
 * sys/attribs.h. 
 ****************************************************************************/



/**
 * @Function someFunction(void)
 * @param foo, some value
 * @return TRUE or FALSE
 * @brief 
 * @note 
 * @author <Your Name>
 * @modified <Your Name>, <year>.<month>.<day> <hour> <pm/am> */



// initializes buffers, returns buffer
void buff_init(buff *buffinit) {
    //buff *buffinit = malloc(sizeof(BuffSize)); // breaks here too 
    buffinit->head = 0;
    buffinit->tail = 0;
//    return buffinit;
}
// write item to buffer


// Returns whether buffer is full, empty, or there was an error reading it
int buffState(buff *buffer3) {
    if ((((buffer3->head) + 1) % BuffSize) == buffer3->tail) { // fucks up here
        return FULL;
    } else if ((buffer3->tail) == (buffer3->head)) { // does not properly check if head and tail are equal?
        return EMPTY;
    }
    return FULL_NOT;
}

// adds item to buffer 
int buff_add(buff *buffer1, uint8_t added) {
    // when head not to pass tail with an addition
    if (buffState(buffer1) != FULL) {
        buffer1->buffArray[buffer1->head] = added; // add item to buffer at head
        buffer1->head = ((buffer1->head) + 1) % BuffSize; // increment & modulo to wrap
        return SUCCESS;
    } else {
        return FULL;
    }
}


// save item at tail, moves tail and returns item from tail
int buff_read(buff *buffer1, char *ch) {
    // 
    if (buffState(buffer1) != EMPTY) {
    //save item at tail
        *ch = buffer1->buffArray[buffer1->tail];
        // move tail
        buffer1->tail = ((buffer1->tail) + 1) % BuffSize;
        return SUCCESS;
    } else {
        return ERROR;
    }
}

// dequeue
int GetChar(unsigned char *ch) {
    return buff_read(&RXbuff, ch);
}
// enqueue
int PutChar(char ch){
    putTrans = TRUE;
    if(buffState(&TXbuff) == FULL) {
        return ERROR;
    }
    
    if (buff_add(&TXbuff, ch) == SUCCESS) {
            putTrans = FALSE;
            TXcollision = FALSE; 
        } else {
            putTrans = FALSE;
            return ERROR;
    
    }
    if (U1STAbits.TRMT) { //  if Transmit shift register is empty and transmit buffer is empty
        IFS0bits.U1TXIF = 1; // then enable flag for transmission
    }
    if (TXcollision == TRUE) {
        TXcollision = FALSE;
        IFS0bits.U1TXIF = 1;
    }
    return SUCCESS;
    // IEC0bits.U1TXIE = 1; // why did i have this here?
}

void Uart_Init(unsigned long baudRate) {
    // First, initialize circular buffers
    buff_init(&TXbuff);
    buff_init(&RXbuff);
    
    U1MODE = 0x00; // Clear Uart for 8-bit Data, no Parity, and 1 Stop bit
    U1STA = 0x00; // Clear Transmitter and Receiver

    U1MODEbits.ON = 1; // Copied from datasheet?


    U1BRG = ((40000000) / (16 * baudRate)) - 1; // Set Uart baud rate(115200))

    U1MODEbits.PDSEL = 0; // parity
    U1MODEbits.STSEL = 0; // stop selection bit
    //U1MODE = 0x8000;// Enable Uart for 8-bit Data, no Parity, and 1 Stop bit

    U1STAbits.URXEN = 1;
    U1STAbits.UTXEN = 1;
    //U1STA = 0x1400;// Enable Transmitter and Receiver
    U1STAbits.UTXISEL = 0b00; //sets condition: interrupt when no space in TX
    U1STAbits.URXISEL = 0b00; //sets condition: interrupt when RX has >= 1 space


    // Configure UART interrupt for both RX and TX
    IEC0bits.U1RXIE = 1; //enable RX interrupt
    IEC0bits.U1TXIE = 1; //enable TX interrupt
    
    IPC6bits.U1IP = 4; //set UART interrupt priority to 4 
   IPC6bits.U1IS = 0; //set interrupt subpriority to 0
    
    IFS0bits.U1RXIF = 0; // set flag for RX register to 0
    IFS0bits.U1TXIF = 0; // set flag for TX register to 0

}

// Untested/Unused
int printU(const char *format) {
    char test[128];
    sprintf(test, format);
    unsigned char testLen = strlen(test);
    
    for (int i = 0; i < testLen-1; i++) {
        if (PutChar(test[i])==ERROR){
        return ERROR;
        }
    }
}


void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
    // if receive flag is set, handle received character input
    if (IFS0bits.U1RXIF) {
        // clear interrupt flag
        IFS0bits.U1RXIF = 0;
        // while receiving buffer is has data in it 
        while (U1STAbits.URXDA == 1) {
            // Write U1RXREG() to buffer
            // buff_add
            buff_add(&RXbuff, U1RXREG);
        }

        // OERR: Receive Buffer Overrun Error Status bit.
        if (U1STAbits.OERR == 1) { // if up
            U1STAbits.OERR = 0; // set back down
        }

        
    }
    // when transmission buffer has open space
    if (IFS0bits.U1TXIF) {
        // Clear the interrupt flag
        IFS0bits.U1TXIF = 0;
        if (putTrans == TRUE) {
            TXcollision = TRUE;
        } else {
            unsigned char foo;
            while (!U1STAbits.UTXBF && buffState(&TXbuff) != EMPTY) { //while TX buff not full? &
                buff_read(&TXbuff, &foo);
                U1TXREG = foo;
        
            }
        }
    }   

}