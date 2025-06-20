/* Template ver 1.0 W2022
 * File: PingSensor.c
 * Author: Nathan Tran
 * Brief: 
 * Created on 
 * Modified on <month> <day>, <year>, <hour> <pm/am
 */

#include "BOARD.h"
#include "FreeRunningTimer.h"
#include "uart.h"
#include "Protocol2.h"
#include "PingSensor.h"
#include <xc.h>

#define TRIGGER_LAT LATBbits.LATB0  // Define trigger pin
#define ECHO_PORT PORTBbits.RB1     // Define echo pin

static volatile unsigned int startTime = 0;
static volatile unsigned int endTime = 0;
static volatile int captureComplete = 0;

int PingSensor_Init(void) {
    // Configure trigger pin as output
    TRISBbits.TRISB0 = 0;
    TRIGGER_LAT = 0;
    
    // Configure echo pin as input
    TRISBbits.TRISB1 = 1;
    
    // Configure Timer2 for timing measurement
    T2CON = 0x8030;  // Enable Timer2, 1:256 prescaler
    TMR2 = 0;
    
    // Configure Input Capture 3 (IC3) for echo signal
    IC3CON = 0x0002;  // Capture on every edge
    IFS0bits.IC3IF = 0;
    IEC0bits.IC3IE = 1;
    
    return 0; // SUCCESS
}

void PingSensor_Trigger(void) {
    TRIGGER_LAT = 1;
    __delay_us(10);
    TRIGGER_LAT = 0;
}

unsigned short PingSensor_GetDistance(void) {
    if (captureComplete) {
        unsigned int pulseWidth = endTime - startTime;
        captureComplete = 0;
        return (unsigned short)(pulseWidth / 58);  // Convert to mm
    }
    return 0;
}

void __ISR(_INPUT_CAPTURE_3_VECTOR, IPL7AUTO) IC3InterruptHandler(void) {
    if (IC3CONbits.ICBNE) {
        unsigned int time = IC3BUF;
        if (!startTime) {
            startTime = time;
        } else {
            endTime = time;
            captureComplete = 1;
            startTime = 0;
        }
    }
    IFS0bits.IC3IF = 0;
}
