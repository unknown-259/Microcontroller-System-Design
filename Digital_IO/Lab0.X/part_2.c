/* Author: Nathan Tran
 * File: part_2.c
 * Date: January 22, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include "BOARD.h"
#include <xc.h>

#define NOPS_FOR_5MS 2000

// Function to create a 5ms delay using NOP instructions
void NOP_delay(void) {
    for (int i = 0; i < NOPS_FOR_5MS; i++) {
        asm("nop");
    }
}

// Function to handle LED counting and button resets
void update_LEDs_and_counter(void) {
    static uint8_t LEDbit = 0x00;
    static int loop_count = 0;

    loop_count++;
    if (loop_count >= 50) {  // 50 iterations for 250ms delay
        LEDbit++;
        loop_count = 0;
    }

    // Update LED display
    LATE = LEDbit;

    // Check if any button is pressed and reset the counter if so
    if ((PORTF & 0x02) | (PORTD & 0x20) | (PORTD & 0x40) | (PORTD & 0x80)) {
        LEDbit = 0x00;
    }

    // If the LEDs are all on, reset to 0
    if (LEDbit == 0xFF) {
        LEDbit = 0x00;
    }
}

int main(int argc, char** argv) {
    TRISE = 0x00;  // Set all pins on port E to output for LEDs
    LATE = 0x00;   // Initially turn off all LEDs

    while (1) {
        update_LEDs_and_counter();  // Update LEDs and the counter
        NOP_delay();  // Delay for 5ms
    }

    return (EXIT_SUCCESS);
}
