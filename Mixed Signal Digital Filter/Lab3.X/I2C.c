/* 
 * File:   I2C.c
 * Author: Nathan Tran
 * Brief: 
 * Created on March 07 2025 12:22 pm
 * Modified on <month> <day>, <year>, <hour> <pm/am>
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/
#include "I2C.h"           // The header file for this source file. 
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "BOARD.h"
#include <sys/attribs.h>
/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define EEPROM_ADDRESS 0x50
/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS IMPLEMENTATIONS                                                *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS                                             *
 ******************************************************************************/

unsigned int I2C_Init(unsigned int Rate){
    I2C1CON = 0;
    I2C1CONbits.A10M = 0;  
}

unsigned char I2C_ReadRegister(unsigned char I2CAddress,unsigned char deviceRegisterAddress){  
    I2C1ADD = I2CAddress;
}