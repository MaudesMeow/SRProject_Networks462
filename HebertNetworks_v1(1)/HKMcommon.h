#ifndef HKMCOMMON_H
#define HKMCOMMON_H

#include <stdio.h>
#include <string>

#define PORT_NUMBER 9021    // Default port number
#define WINDOW_SIZE 16      // Default window size
#define PACKET_SIZE 1024    // Default packet size
#define SEQUENCE_SIZE 64    // Default sequence number size
#define USE_POLL false      // for testing
#define TESTING true        // makes prompts easier for us

/* CRC generation code is modified from Michael Barr's open source code:
 *  https://barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
 */
typedef unsigned int crc;

#define POLYNOMIAL 0x04C11DB7
#define INITIAL_REMAINDER 0xFFFFFFFF
//#define FINAL_XOR_VALUE 0xFFFFFFFF // might need this if CHECK_VALUE doesn't match
#define CHECK_VALUE 0xCBF43926
#define CRCBYTES sizeof(crc)
#define CRCWIDTH (8 * sizeof(crc))
#define CRCTOPBIT (1 << (CRCWIDTH - 1))

void crcTableInit();
crc crcFun(unsigned char const message[], int nBytes);

int UserInputPromptPort();
std::string UserInputPromptFile(std::string prompt);


#endif