/* Author: Nathan Tran
 * File: part_1.c
 * Date: January 22, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include "BOARD.h"
#include <xc.h>

// Function to update LEDs based on button inputs
void update_LEDs(void) {
    if (PORTF & 0x02) {         // PORTF bit 2 is btn1
        LATE = 0b10000001;
        
    } else if (PORTD & 0x20) {  // PORTD bit 5 is btn2
        LATE = 0b01000010;
        
    } else if (PORTD & 0x40) {  // PORTF bit 6 is btn3
        LATE = 0b00100100;
        
    } else if (PORTD & 0x80) {  // PORTF bit 7 is btn4
        LATE = 0b00011000;
        
    } else {
        LATE = 0b00000000;      // all LEDs off
    }
}

int main(int argc, char** argv) {
    TRISE = 0x00;  // Set all LEDs to output
    LATE = 0x00;   // Turn off all LEDs initially

    while (1) {
        update_LEDs();  // Call the function to check button states and update LEDs
    }

    return (EXIT_SUCCESS);
}
