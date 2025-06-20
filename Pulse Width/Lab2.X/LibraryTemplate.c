/* Template ver 1.0 W2022
 * File:   LibraryTemplate.c
 * Author: <Your Name>
 * Brief: 
 * Created on <month> <day>, <year>, <hour> <pm/am>
 * Modified on <month> <day>, <year>, <hour> <pm/am
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/

#include "LibraryTemplate.h" // The header file for this source file. 

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define baudrate ((80000000)/(115200))-1

/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                 *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS                                             *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS                                            *
 ******************************************************************************/



/**
 * @Function someFunction(void)
 * @param foo, some value
 * @return TRUE or FALSE
 * @brief 
 * @note 
 * @author <Your Name>
 * @modified <Your Name>, <year>.<month>.<day> <hour> <pm/am> */
uint8_t UARTinit(int foo){
    U1MODESET= 0x00;// Clear Uart for 8-bit Data, no Parity, and 1 Stop bit
    U1STASET= 0x00;// Clear Transmitter and Receiver
    
    
    U1MODESET= 0x8000;// Enable Uart for 8-bit Data, no Parity, and 1 Stop bit
    U1STASET= 0x1400;// Enable Transmitter and Receiver
    
    U1BRG = baudrate;// Set Uart baud rate(115200))
 }