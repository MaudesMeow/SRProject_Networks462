#ifndef HKMCOMMON_HPP
#define HKMCOMMON_HPP

#include <stdio.h>
#include <string>

#define PORT_NUMBER         9021            // Default port number
#define WINDOW_SIZE         16              // Default window size
#define PACKET_SIZE         1024            // Default packet size
#define SEQUENCE_SIZE       64              // Default sequence number size
#define TESTING             true            // makes prompts easier for us
#define HEADER_SIZE         sizeof(int)*3   // for use in sending and receiving
#define BYTES_OF_PADDING    8               // 4 for the sequence number and 4 for the crc (both ints)
#define KILLCODE            20000000        // send this to the server so it knows the file is done (20 million)


/* CRC generation code is modified from Michael Barr's open source code:
 *  https://barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
 */
typedef unsigned int crc;

#define POLYNOMIAL 0x04C11DB7
#define INITIAL_REMAINDER 0xFFFFFFFF
#define CHECK_VALUE 0xCBF43926
#define CRCBYTES sizeof(crc)
#define CRCBITS (8 * CRCBYTES)
#define CRCTOPBIT (1 << (CRCBITS - 1))

void crcTableInit();
crc crcFun(char *message, int nBytes);

int UserInputPromptPort();
std::string UserInputPromptFile(std::string prompt);
int UserInputPromptErrorGenerationMethod(std::string errorType);
int UserInputPromptErrorCount(std::string errorType);
int *UserInputPromptGenerateErrorArray(int count, std::string errorType);
int randomGeneratedErrorCount();
int *randomGeneratedErrorArray(int count);

#endif