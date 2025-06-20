/* Template ver 1.0 W2022
 * File: RotaryEncoder.c
 * Author: Nathan Tran
 * Brief: 
 * Created on 
 * Modified on <month> <day>, <year>, <hour> <pm/am
 */

#include <xc.h> // XC compiler definitions
#include <sys/attribs.h> // System attributes
#include <stdio.h> // Standard I/O functions

#include "RotaryEncoder.h" // Header file for rotary encoder functions
#include "MessageIDs.h" // Message ID definitions
#include "Protocol2.h" // Protocol functions for communication

#define PBCLK 40000000 // Peripheral Bus Clock frequency (40 MHz, derived from 80 MHz / 2)
#define AS5047D_READ_ANGLE_CMD 0xFFFF // Command to read the angle from the AS5047D encoder

#define CS LATDbits.LATD0 // Chip Select pin for SPI communication

static int rData; // Variable to store SPI read data

/**
 * @Function NOP_delay_1us
 * @param a - Number of microseconds to delay
 * @brief Creates a delay using NOP (No Operation) instructions for microseconds.
 * @note This function is used for short, precise delays.
 */
void NOP_delay_1us(int a) {
    for (int i = 0; i < a; i++) {
        for (int j = 0; j < 180; j++) {
            asm("nop"); // No operation instruction for delay
        }
    }
}

/**
 * @Function NOP_delay_1ms
 * @param a - Number of milliseconds to delay
 * @brief Creates a delay using NOP (No Operation) instructions for milliseconds.
 * @note This function is used for longer delays.
 */
void NOP_delay_1ms(int a) {
    for (int i = 0; i < a; i++) {
        for (int j = 0; j < 80000; j++) {
            asm("nop"); // No operation instruction for delay
        }
    }
}

/**
 * @Function SPI_Init
 * @brief Initializes the SPI1 module for communication with the AS5047D rotary encoder.
 * @note Configures SPI1 in 16-bit mode with a clock frequency of 10 MHz.
 */
void SPI_Init(void) {
    // Configure SPI1 pins
    TRISGbits.TRISG6 = 0; // Set RG6 (SCK1, pin 38) as output (CLK)
    TRISFbits.TRISF2 = 1; // Set RF2 (SDI1, pin 60) as input (MISO)
    TRISFbits.TRISF3 = 0; // Set RF3 (SDO1, pin 1) as output (MOSI)
    TRISBbits.TRISB0 = 0; // Set RD0 (CS, pin 3) as output (CSn)

    SPI1CON = 0; // Stop and reset SPI1

    rData = SPI1BUF; // Clear the receive buffer by reading from it

    SPI1BRG = 0x1; // Set Baud Rate register for 10 MHz SPI clock (40 MHz / (2*(1+1)))

    SPI1STATCLR = 0x40; // Clear the overflow flag
    SPI1CONbits.CKE = 1; // Clock edge: Mode 1 (CPOL=0, CPHA=1)
    SPI1CONbits.MODE16 = 1; // Enable 16-bit mode
    SPI1CONbits.SMP = 1; // Sample input data at the end of the data output time
    SPI1CONbits.MSTEN = 1; // Enable master mode
    SPI1CONbits.ON = 1; // Enable SPI1 module
}

/**
 * @Function RotaryEncoder_Init
 * @param interfaceMode - Mode of operation (blocking or interrupt-based)
 * @return SUCCESS or ERROR
 * @brief Initializes the rotary encoder hardware and SPI interface.
 * @note This function sets up the encoder for communication in the specified mode.
 */
int RotaryEncoder_Init(char interfaceMode) {
    SPI_Init(); // Initialize SPI for encoder communication
    return 1; // Return success
}

/**
 * @Function SPI1_Read
 * @param reg - Register address to read from
 * @return Data read from the specified register
 * @brief Reads data from the SPI1 buffer.
 * @note This function waits for the transmit and receive buffers to be ready.
 */
unsigned short SPI1_Read(unsigned short reg) {
    while (!SPI1STATbits.SPITBE); // Wait until the transmit buffer is empty
    SPI1BUF = reg; // Write the register address to the buffer
    while (!SPI1STATbits.SPIRBF); // Wait until the receive buffer is full
    return SPI1BUF; // Return the received data
}

/**
 * @Function RotaryEncoder_ReadRawAngle
 * @param None
 * @return 14-bit raw angle value from the encoder (0-16384)
 * @brief Reads the raw angle value from the AS5047D rotary encoder.
 * @note This function communicates with the encoder using SPI and returns the angle.
 */
unsigned short RotaryEncoder_ReadRawAngle(void) {
    CS = 0; // Activate chip select (active low)
    unsigned short raw_angle = SPI1_Read(AS5047D_READ_ANGLE_CMD) & 0x3FFF; // Read angle and mask out parity bits
    CS = 1; // Deactivate chip select (active low)
    return raw_angle; // Return the raw angle value
}

// Test harness for rotary encoder functionality
#define ROT_ENC_TEST
#ifdef ROT_ENC_TEST

/**
 * @Function main
 * @param argc - Argument count
 * @param argv - Argument vector
 * @return 0 on successful execution
 * @brief Main function to test the rotary encoder functionality.
 * @note This function initializes the protocol and encoder, then continuously reads and sends the angle.
 */
int main(int argc, char** argv) {
    Protocol_Init(115200); // Initialize protocol with a baud rate of 115200
    RotaryEncoder_Init(ENCODER_BLOCKING_MODE); // Initialize the rotary encoder in blocking mode

    char messageComp[100]; // Buffer for debug message
    sprintf(messageComp, "[DEBUG] Rotary Encoder test harness compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(messageComp); // Send compilation timestamp as a debug message

    while(1) {
        unsigned short angle = RotaryEncoder_ReadRawAngle(); // Read the raw angle from the encoder
        unsigned short disp_angle = (angle & 0xFF00) >> 8 | (angle & 0x00FF) << 8; // Convert to big-endian format
        Protocol_SendPacket(3, ID_ROTARY_ANGLE, (unsigned char*)&disp_angle); // Send the angle as a packet
        NOP_delay_1ms(10); // Delay for 10 milliseconds
    }

    return 0; // Return success
}

#endif