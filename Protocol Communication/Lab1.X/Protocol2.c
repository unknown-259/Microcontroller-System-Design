/* Template ver 1.0 W2022
 * File:   Protocol2.c
 * Author: Nathan Tran
 * Brief: 
 * Created on January 28, 2025, 2:48 pm
 * Modified on <month> <day>, <year>, <hour> <pm/am>
 */

/*******************************************************************************
 * #INCLUDES                                                                   *
 ******************************************************************************/

#include "Protocol2.h" // Protocol header file
#include "uart.h" // UART communication functions
#include <stdio.h> // Standard input/output library for sprintf
#include "BOARD.h" // Board initialization and control
#include <xc.h> // Microcontroller-specific includes for system configuration
#include "sys/attribs.h" // Attribute macros for interrupts
#include "MessageIDs.h" // Defines for message IDs
#include <stdint.h> // Standard integers like uint8_t, uint16_t
#include <string.h> // String handling functions like strlen

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define HEAD 0xCC // Start byte for packets
#define TAIL 0xB9 // End byte for packets
#define baudRate 115200 // Communication baud rate for UART

/*******************************************************************************
 * PRIVATE TYPEDEFS                                                            *
 ******************************************************************************/
typedef enum {
    STANDBY, READ_len, READ_ID, READ_payload, READ_checksum, BUILD
} STATES;

static int PktReady = 0; // Flag indicating a packet is ready to be processed
static rxpADT ProtocolPacket; // Struct for the received protocol packet

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
static packBuff packets; // Circular buffer for storing packets
static unsigned char cksum; // Checksum for received data
static unsigned char cksumSend; // Checksum for sending data
static unsigned char cksumRecieve; // Checksum for received packets

// Initialize the protocol, setting up UART and LEDs
int Protocol_Init(unsigned long baudrate) {
    Uart_Init(baudrate); // Initialize UART with the specified baud rate
    packets.head = 0; // Initialize packet buffer head
    packets.tail = 0; // Initialize packet buffer tail
    LEDS_INIT(); // Initialize LEDs on the board
    LEDS_SET(0x00); // Turn off all LEDs by default
    state = STANDBY; // Reset state machine to standby
    cksumSend = 0;
    cksumRecieve = 0;
    char message[MAXPAYLOADLENGTH];
    sprintf(message, "Nathan Tran's protocol.c");
    Protocol_SendDebugMessage(message);
}

// Unused
uint8_t Protocol_QueuePacket(void);
int Protocol_GetInPacket(uint8_t *type, uint8_t *len, unsigned char *msg);

// Send a debug message with the current compilation timestamp
int Protocol_SendDebugMessage(char *Message) {
    char debugArray[MAXPAYLOADLENGTH];
    sprintf(debugArray, "%s  -- Compiled on %s %s\n", Message, __DATE__, __TIME__);
    unsigned char msgLen = strlen(debugArray);
    Protocol_SendPacket(msgLen, ID_DEBUG, debugArray);
    return SUCCESS;
}

// Send a protocol packet with the specified ID and payload
int Protocol_SendPacket(unsigned char len, unsigned char ID, unsigned char *Payload) {    
    // Sending a full message consisting of HEAD, length, ID, payload, TAIL, checksum, END
    if (PutChar(HEAD) == ERROR) return ERROR;
    if (PutChar(len) == ERROR) return ERROR;
    if (PutChar(ID) == ERROR) return ERROR;
    cksumSend = BSDChecksum(cksumSend, ID);

    int check;
    for (int i = 0; i < len - 1; i++) {
        if (PutChar(Payload[i]) == ERROR) return ERROR;
        cksumSend = BSDChecksum(cksumSend, Payload[i]);
        check = i;
    }

    if (PutChar(TAIL) == ERROR) return ERROR;
    if (PutChar(cksumSend) == ERROR) return ERROR;
    PutChar('\r'); // End message with carriage return
    PutChar('\n'); // End message with newline
    cksumSend = 0; // Clear checksum after sending
    return (len == check + 1) ? SUCCESS : ERROR;
}

// Unused
unsigned char Protocol_ReadNextPacketID(void);
void flushPacketBuffer(void);

// Function to swap the endianness of a 32-bit integer (currently unused)
unsigned int convertEndian(unsigned int num) {
    unsigned int swapped = ((num >> 24) & 0xff) | // Byte 3 to byte 0
                           ((num << 8) & 0xff0000) | // Byte 1 to byte 2
                           ((num >> 8) & 0xff00) | // Byte 2 to byte 1
                           ((num << 24) & 0xff000000); // Byte 0 to byte 3
    return swapped;
}

/**
 * @Function someFunction(void)
 * @param rxPacket, pointer packet built by this function
 * @return TRUE or FALSE
 * @brief 
 * @note 
 * @author Nathan Tran
 * @modified <Your Name>, <year>.<month>.<day> <hour> <pm/am> */
uint8_t BuildRxPacket(rxpADT *rxPacket, unsigned char ch, unsigned char reset) {
    static int i;

    switch (state) {
        case STANDBY:
            i = 0; // Reset counter for payload
            if (ch == HEAD) state = READ_len;
            break;
        case READ_len:
            rxPacket->len = ch; // Store packet length
            state = READ_ID;
            break;
        case READ_ID:
            rxPacket->ID = ch; // Store packet ID
            cksumRecieve = BSDChecksum(cksumRecieve, ch); // Update checksum
            state = READ_payload;
            break;
        case READ_payload:
            if (i < (rxPacket->len) - 1) {
                rxPacket->payLoad[i] = ch; // Store payload data
                cksumRecieve = BSDChecksum(cksumRecieve, ch); // Update checksum
                i++;
            } else {
                state = READ_checksum;
            }
            break;
        case READ_checksum:
            rxPacket->checkSum = ch; // Store received checksum
            if (rxPacket->checkSum == cksumRecieve) {
                state = BUILD; // Proceed to build packet if checksum matches
            } else {
                state = STANDBY; // Discard packet if checksum fails
            }
            cksumRecieve = 0; // Clear checksum after processing
            break;
        case BUILD:
            if (packState(&packets) != ERROR) {
                packets.packItems[packets.head] = *rxPacket; // Add packet to buffer
                packets.head = (packets.head + 1) % PACKETBUFFERSIZE;
                state = STANDBY; // Reset state machine
                PktReady = 1; // Set packet ready flag
            }
            break;
    }
}

// Parse the packet and execute commands based on its ID
uint8_t Protocol_ParsePacket(void) {
    if (packets.tail == packets.head) { // No packets to process
        return ERROR;
    }

    rxpADT parsePack = packets.packItems[packets.tail]; // Get next packet
    packets.tail = ((packets.tail)+1) % PACKETBUFFERSIZE;   // Update tail pointer
    uint8_t LEDS_ARE;
    
    switch (parsePack.ID) {
        case ID_INVALID:
            PktReady = 0;
            break;
        case ID_LEDS_SET:
            LEDS_SET(parsePack.payLoad[0]); // Set LEDs based on payload
            PktReady = 0;
            break;
        case ID_LEDS_GET:
            LEDS_ARE = LEDS_GET();  // Get current LED state
            uint8_t LED_array[1];
            LED_array[0] = LEDS_ARE;
            Protocol_SendPacket(0x02, ID_LEDS_STATE, LED_array); 
            PktReady = 0;
            break;
        case ID_PING:
            ; // idk why this works but it fixes a declaration error in the next line
            // Put Payload Array into unsigned int 
            unsigned int preEndLoad;
            
            // Turns payload array into single value, eg: [0,0,0,1] to 0001
            for (int k = 0; k < (parsePack.len-1); k++) { // Also converts from Big to Little Endian
                preEndLoad *= 256; // Add a 00 in hex
                preEndLoad += parsePack.payLoad[k];
            }
 
            unsigned int postEndLoad = preEndLoad; 
            postEndLoad = postEndLoad >> 1; // Right shift to divide by 2
            unsigned char pongArray[4]; // Make array again to send as payload out
            
            pongArray[3] = postEndLoad; // Also converts from Little to Big Endian
            pongArray[2] = postEndLoad >> 8;
            pongArray[1] = postEndLoad >> 16;
            pongArray[0] = postEndLoad >> 24;
            
            Protocol_SendPacket(0x5, ID_PONG, pongArray); // 1 byte ID, 4 bytes
            PktReady = 0;
            
            break;
    }
}

// Main that uses protocol functions to receive and send packets
void runProtocol() {
    static unsigned char reset;
    unsigned char ch;

    while (1) {

        if (GetChar(&ch) == SUCCESS) {
            BuildRxPacket(&ProtocolPacket, ch, reset);
        }
        while (PktReady == 1) {
            Protocol_ParsePacket();
        }
    }
}

// Unused
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum);

// Function to calculate the BSD checksum for data integrity
unsigned char BSDChecksum(unsigned char cksum, unsigned char str) {
    cksum = (cksum >> 1) + (cksum << 7); // Circular rotation
    cksum += str; // Add byte to checksum
    return cksum;
}

// Check the state of the packet buffer (whether it's full or empty)
int packState(packBuff *buffer3) {
    if ((((buffer3->head) + 1) % PACKETBUFFERSIZE) == buffer3->tail) {
        return ERROR; // Buffer is full
    } else if ((buffer3->tail) == (buffer3->head)) {
        return SUCCESS; // Buffer is empty
    }
    return SUCCESS;
}
