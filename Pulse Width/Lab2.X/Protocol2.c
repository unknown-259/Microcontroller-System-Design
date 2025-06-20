/* Template ver 1.0 W2022
 * File:   Protocol2.c
 * Author: Nathan Tran
 * Brief: 
 * Created on 
 * Modified on <month> <day>, <year>, <hour> <pm/am
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/

#include "Protocol2.h" // The header file for this source file.
#include "uart.h" // UART communication functions
#include <stdio.h> // Standard I/O functions
#include "BOARD.h" // Board-specific definitions
#include <xc.h> // XC compiler definitions
#include "sys/attribs.h" // System attributes
#include "MessageIDs.h" // Message ID definitions
#include <stdio.h> // Standard I/O functions (included again for redundancy)
#include <stdint.h> // Standard integer types
#include <string.h> // String manipulation functions

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define HEAD 0xCC // Packet header byte
#define TAIL 0xB9 // Packet tail byte
#define baudRate 115200 // Baud rate for UART communication

/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/
typedef enum {
    STANDBY, READ_len, READ_ID, READ_payload, READ_checksum, BUILD
} STATES; // State machine states for packet processing

static int PktReady = 0; // Flag to indicate if a packet is ready for processing
static rxpADT ProtocolPacket; // Structure to hold the received packet

/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS                                             *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS                                            *
 ******************************************************************************/

/*******************************************************************************
 * GLOBAL DATA                                                                 *
 ******************************************************************************/
static int state = STANDBY; // Current state of the state machine
static packBuff packets; // Buffer to hold incoming packets
static unsigned char cksum; // Checksum for packet validation
static unsigned char cksumSend; // Checksum for sending packets
static unsigned char cksumRecieve; // Checksum for receiving packets

/**
 * @Function BSDChecksum
 * @param cksum - Current checksum value
 * @param str - Byte to add to the checksum
 * @return Updated checksum value
 * @brief Calculates the BSD checksum for a given byte.
 * @note This function performs a circular rotation and adds the byte to the checksum.
 */
unsigned char BSDChecksum(unsigned char cksum, unsigned char str) {
    cksum = (cksum >> 1) + (cksum << 7); // Circular rotation
    cksum += str; // Add byte to checksum
    return cksum;
}

/**
 * @Function Protocol_Init
 * @param baudrate - Baud rate for UART communication
 * @return SUCCESS or ERROR
 * @brief Initializes the protocol, including UART and state machine.
 * @note This function sets up the UART, initializes the packet buffer, and resets the state machine.
 */
int Protocol_Init(unsigned long baudrate) {
    Uart_Init(baudrate);
    packets.head = 0; // Initialize packet buffer head
    packets.tail = 0; // Initialize packet buffer tail
    LEDS_INIT(); // Initialize LEDs
    LEDS_SET(0x00); // Turn off all LEDs by default
    state = STANDBY; // Reset state machine to STANDBY
    cksumSend = 0; // Reset send checksum
    cksumRecieve = 0; // Reset receive checksum
}

// UNUSED FUNCTIONS
// These functions were defined but not used in the final implementation.
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum);
uint8_t Protocol_QueuePacket(void); // Unused, done in build state manually
int Protocol_GetInPacket(uint8_t *type, uint8_t *len, unsigned char *msg); // Unused, part of state machine, different for each
unsigned char Protocol_ReadNextPacketID(void); // Unused, idk why use this
void flushPacketBuffer(void); // Unused, i change state or lose packet when error

/**
 * @Function Protocol_SendDebugMessage
 * @param Message - Debug message to send
 * @return SUCCESS or ERROR
 * @brief Sends a debug message with compilation timestamp.
 * @note This function formats the debug message with the compilation date and time.
 */
int Protocol_SendDebugMessage(char *Message){
    char debugArray[MAXPAYLOADLENGTH];
    sprintf(debugArray, "%s  -- Compiled on %s %s\n", Message, __DATE__, __TIME__);
    unsigned char msgLen = strlen(debugArray);
    Protocol_SendPacket(msgLen, ID_DEBUG, debugArray);
    return SUCCESS;
}

/**
 * @Function Protocol_SendPacket
 * @param len - Length of the payload
 * @param ID - Message ID
 * @param Payload - Payload data
 * @return SUCCESS or ERROR
 * @brief Sends a packet with the specified length, ID, and payload.
 * @note This function constructs and sends a packet, including header, tail, and checksum.
 */
int Protocol_SendPacket(unsigned char len, unsigned char ID, unsigned char *Payload) {    
    if (PutChar(HEAD)==ERROR){
        return ERROR;
    }
    if (PutChar(len)==ERROR){
        return ERROR;
    }
    if (PutChar(ID)==ERROR){
        return ERROR;
    }
    cksumSend = BSDChecksum(cksumSend, ID);
    
    int check;
    for (int i = 0; i < len-1; i++) {
        if (PutChar(Payload[i])==ERROR){
            return ERROR;
        }
        cksumSend = BSDChecksum(cksumSend, Payload[i]);
        check = i;
    }
    
    if (PutChar(TAIL)==ERROR){
        return ERROR;
    }
    if (PutChar(cksumSend)==ERROR){
        return ERROR;
    }
    PutChar('\r');
    PutChar('\n');
    cksumSend = 0; // Clear checksum
    if (len = check + 1) {
        return SUCCESS;
    } else {
        return ERROR;
    }
}

/**
 * @Function convertEndian
 * @param num - Number to convert
 * @return Swapped endian value
 * @brief Converts the endianness of a 32-bit integer.
 * @note This function swaps the byte order of the input number.
 */
unsigned int convertEndian(unsigned int num) {
    unsigned int swapped =((num>>24)&0xff) | // move byte 3 to byte 0
            ((num<<8)&0xff0000) | // move byte 1 to byte 2
            ((num>>8)&0xff00) | // move byte 2 to byte 1
            ((num<<24)&0xff000000); // byte 0 to byte 3
    return swapped;
}

/**
 * @Function packState
 * @param buffer3 - Pointer to the packet buffer
 * @return SUCCESS or ERROR
 * @brief Checks the state of the packet buffer.
 * @note This function returns ERROR if the buffer is full, otherwise SUCCESS.
 */
int packState(packBuff *buffer3) {
    if ((((buffer3->head) + 1) % PACKETBUFFERSIZE) == buffer3->tail) { 
        return ERROR;
    } else if ((buffer3->tail) == (buffer3->head)) { 
        return SUCCESS;
    }
    return SUCCESS;
}

/**
 * @Function BuildRxPacket
 * @param rxPacket - Pointer to the packet structure
 * @param ch - Incoming byte
 * @param reset - Reset flag
 * @return TRUE or FALSE
 * @brief Builds a received packet from incoming bytes.
 * @note This function uses a state machine to process incoming bytes into a packet.
 */
uint8_t BuildRxPacket(rxpADT *rxPacket, unsigned char ch, unsigned char reset) {
    static int i;
    
    switch (state) {
    case STANDBY:
        i = 0; // Reset counter for payload
        if (ch == HEAD) {
            state = READ_len;
            break;
        } else {
            break;
        }
    case READ_len:
        rxPacket->len = ch;
        state = READ_ID;
        break;
    case READ_ID:
        rxPacket->ID = ch;
        cksumRecieve = BSDChecksum(cksumRecieve, ch);
        state = READ_payload;
        break;

    case READ_payload:
        if (i < (rxPacket->len) - 1) {
            rxPacket->payLoad[i] = ch;
            cksumRecieve = BSDChecksum(cksumRecieve, ch);
            i++;
            break;
        }
        state = READ_checksum;
        break;

    case READ_checksum:
        rxPacket->checkSum = ch;
        if (rxPacket->checkSum == cksumRecieve) {
            state = BUILD;
        } else {
            state = STANDBY;
        }
        cksumRecieve = 0; // Clear checksum
        break;

    case BUILD:
        if (packState(&packets) != ERROR) {
            packets.packItems[packets.head] = *rxPacket;
            packets.head = ((packets.head)+1) % PACKETBUFFERSIZE;
            state = STANDBY;
            PktReady = 1;
            break;
        }
    }
}

/**
 * @Function Protocol_ParsePacket
 * @return SUCCESS or ERROR
 * @brief Parses the received packet and performs actions based on the packet ID.
 * @note This function processes the packet and handles different message types.
 */
uint8_t Protocol_ParsePacket(void) {
    if (packets.tail == packets.head) {
        return ERROR;
    }
    rxpADT parsePack = packets.packItems[packets.tail];
    packets.tail = ((packets.tail)+1) % PACKETBUFFERSIZE;
    uint8_t LEDS_ARE;
    
    switch (parsePack.ID) {
        case ID_INVALID:
            PktReady = 0;
            break;
        case ID_LEDS_SET:
            LEDS_SET(parsePack.payLoad[0]);
            PktReady = 0;
            break;
        case ID_LEDS_GET:
            LEDS_ARE = LEDS_GET();
            uint8_t LED_array[1];
            LED_array[0] = LEDS_ARE;
            Protocol_SendPacket(0x02, ID_LEDS_STATE, LED_array); 
            PktReady = 0;
            break;
        case ID_PING:
            unsigned int preEndLoad;
            for (int k = 0; k < (parsePack.len-1); k++) {
                preEndLoad *= 256;
                preEndLoad += parsePack.payLoad[k];
            }
            unsigned int postEndLoad = preEndLoad;
            postEndLoad = postEndLoad >> 1;
            unsigned char pongArray[4];
            pongArray[3] = postEndLoad;
            pongArray[2] = postEndLoad >> 8;
            pongArray[1] = postEndLoad >> 16;
            pongArray[0] = postEndLoad >> 24;
            Protocol_SendPacket(0x5, ID_PONG, pongArray);
            PktReady = 0;
            break;
    }
}

/**
 * @Function runProtocol
 * @brief Main function to run the protocol for receiving and sending packets.
 * @note This function continuously processes incoming bytes and handles packet parsing.
 */
void runProtocol() {
    static unsigned char reset;
    unsigned char ch;
    
    if (GetChar(&ch) == SUCCESS) {
        BuildRxPacket(&ProtocolPacket, ch, reset);
    }
    while (PktReady == 1) {
        Protocol_ParsePacket();
    }
}