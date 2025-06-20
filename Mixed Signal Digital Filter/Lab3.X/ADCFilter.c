/* 
 * File:   ADCFilter.c
 * Author: Nathan Tran
 * Brief: Implements ADC filtering functionality, including initialization, interrupt handling, and applying a filter to raw readings.
 * Created on 
 * Modified on 
 */

#include "ADCFilter.h"
#include <stdio.h>

// Array to store ADC values for each pin
uint16_t adc_values[4][FILTERLENGTH] = {{0}};

// Array to store filter values for each pin
short filter_values[4][FILTERLENGTH] = {{0}};

// Index to keep track of current ADC reading position
uint16_t idx = 0;

// Function to initialize the ADC filter and configure ADC settings
uint8_t ADCFilter_Init(void) {
    AD1CON1bits.ON = 0; // Turn off ADC during configuration

    // Set analog pins for ADC readings
    AD1PCFGbits.PCFG2 = 0;
    AD1PCFGbits.PCFG4 = 0;
    AD1PCFGbits.PCFG8 = 0;
    AD1PCFGbits.PCFG10 = 0;

    // Configure pins as input
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB10 = 1;

    // Select pins to be scanned by ADC
    AD1CSSLCLR = 0xFFFF;
    AD1CSSLbits.CSSL2 = 1;
    AD1CSSLbits.CSSL4 = 1;
    AD1CSSLbits.CSSL8 = 1;
    AD1CSSLbits.CSSL10 = 1;

    AD1CON1bits.FORM = 0b000; // Set output format to uint16
    AD1CON1bits.SSRC = 0b111; // Set auto conversion on internal timer
    AD1CON1bits.ASAM = 1; // Enable auto sampling

    AD1CON2bits.VCFG = 0b000; // Use default VCC and VSS as voltage reference
    AD1CON2bits.CSCNA = 1; // Enable scanning of inputs
    AD1CON2bits.SMPI = 0b0111; // Interrupt after every 4 conversions
    AD1CON2bits.BUFM = 0; // Use single buffer of size 16

    AD1CON2bits.ALTS = 0; // Use mux A for input selection
    AD1CON3bits.ADRC = 0; // Use PB clock as ADC clock source
    AD1CON3bits.ADCS = 173; // Set ADC clock divisor (formula 2*(ADCS+1))
    AD1CON3bits.SAMC = 16; // Set sample time to 16 clock cycles

    AD1CON1bits.ON = 1; // Turn on the ADC

    // Set up ADC interrupts
    __builtin_disable_interrupts(); // Disable interrupts during setup

    IFS1bits.AD1IF = 0; // Clear interrupt flag
    IPC6bits.AD1IP = 0b0111; // Set interrupt priority
    IPC6bits.AD1IS = 0b11; // Set interrupt sub-priority
    IEC1bits.AD1IE = 1; // Enable ADC interrupt

    __builtin_enable_interrupts(); // Enable interrupts after setup

    return 0;
}

// ADC interrupt handler that stores ADC values in the buffer
void __ISR(_ADC_VECTOR, IPL7SRS) ADCIntHandler(void) {
    IFS1bits.AD1IF = 0; // Clear interrupt flag
    idx = (idx + 1) % FILTERLENGTH; // Update index for next value
    adc_values[0][idx] = ADC1BUF0; // Store ADC result for pin 0
    adc_values[1][idx] = ADC1BUF1; // Store ADC result for pin 1
    adc_values[2][idx] = ADC1BUF2; // Store ADC result for pin 2
    adc_values[3][idx] = ADC1BUF3; // Store ADC result for pin 3
}

// Function to get the raw ADC value for a specified pin
uint16_t ADCFilter_RawReading(uint8_t pin) {
    return adc_values[pin][idx]; // Return raw ADC value for the specified pin
}

// Function to get the filtered ADC value for a specified pin
short ADCFilter_FilteredReading(uint8_t pin) {
    return ADCFilter_ApplyFilter(filter_values[pin], adc_values[pin], idx); // Apply filter and return result
}

// Function to apply a filter to the ADC values and return the filtered result
short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex) {
    int result = 0;

    // Loop through the filter and values arrays to compute the filtered result
    for (uint16_t i = 0; i < FILTERLENGTH; i++) {
        result += ((int)(values[(i + startIndex) % FILTERLENGTH])) 
                    * ((int)(filter[(i + startIndex) % FILTERLENGTH]));
    }

    return result; // Return the filtered result
}

// Function to set the weights for the filter on a specified pin
void ADCFilter_SetWeights(short pin, short weights[]) {
    memcpy(filter_values[pin], weights, FILTERLENGTH * sizeof(short)); // Copy weights into filter array
}
