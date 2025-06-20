/* Author: Nathan Tran
 * File: BOARD.c
 * Date: January 22, 2025
 */

#include "BOARD.h"
#include <xc.h>

#pragma config FPBDIV     = DIV_2
#pragma config FPLLIDIV   = DIV_2
#pragma config IESO       = OFF
#pragma config POSCMOD    = XT
#pragma config OSCIOFNC   = OFF
#pragma config FCKSM      = CSECMD
#pragma config WDTPS      = PS1
#pragma config FWDTEN     = OFF
#pragma config ICESEL     = ICS_PGx2
#pragma config PWP        = OFF
#pragma config BWP        = OFF
#pragma config CP         = OFF
#pragma config FNOSC      = PRIPLL
#pragma config FSOSCEN    = OFF
#pragma config FPLLMUL    = MUL_20
#pragma config FPLLODIV   = DIV_1

/*
 * PRIVATE #DEFINES
 */
#define SYSTEM_CLOCK 80000000L
#define PB_CLOCK (SYSTEM_CLOCK / 2)

static void disable_unused_modules(void);
static void configure_interrupts(void);
static void disable_jtag(void);

void BOARD_Init(void) {
    disable_unused_modules();
    configure_interrupts();
    disable_jtag();
}

void BOARD_End(void) {
    // Disable all interrupts except serial and A/D, and clear their flags
    IEC0CLR = ~(_IEC0_U1TXIE_MASK | _IEC0_U1RXIE_MASK);
    IFS0CLR = ~(_IFS0_U1TXIF_MASK | _IFS0_U1RXIF_MASK);

    IEC1CLR = ~(_IEC1_AD1IE_MASK);
    IFS0CLR = ~(_IFS1_AD1IF_MASK);

    // Set all ports to be digital inputs
    TRISB = 0xFF;
    TRISC = 0xFF;
    TRISD = 0xFF;
    TRISE = 0xFF;
    TRISF = 0xFF;
    TRISG = 0xFF;
}

unsigned int BOARD_GetPBClock(void) {
    return PB_CLOCK;
}

/******************************************************************************
 * PRIVATE HELPER FUNCTIONS                                                   *
 ******************************************************************************/

// Disable unused peripheral modules such as A/D pins for a clean start
static void disable_unused_modules(void) {
    // Disable all A/D pins for a clean start
    AD1PCFG = 0xFFFF;  
}

// Configure interrupts for multi-vector mode and enable global interrupts
static void configure_interrupts(void) {
    unsigned int val;
    
    // Set the CP0 cause IV bit high
    asm volatile("mfc0   %0,$13" : "=r"(val));
    val |= 0x00800000;
    asm volatile("mtc0   %0,$13" : "+r"(val));

    // Enable multi-vector interrupt mode
    INTCONSET = _INTCON_MVEC_MASK;
    
    // Enable global interrupts
    unsigned int status = 0;
    asm volatile("ei    %0" : "=r"(status));
}

// Disable the JTAG interface to free up the pins
static void disable_jtag(void) {
    DDPCONbits.JTAGEN = 0;  // Disable JTAG
}
