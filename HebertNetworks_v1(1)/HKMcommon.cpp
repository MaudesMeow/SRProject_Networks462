

#include <stdio.h>
#include <string>
#include <iostream>
#include "HKMcommon.h"


//prompts te user for the port number
int UserInputPromptPort()
{
        int port;
        std::cout << "Enter port number or -1 for default (" << PORT_NUMBER << "): " << std::endl;
        std::cin >> port;
        if (port < 0)
        {
                port = PORT_NUMBER;
        }
        return port;
}

//returns name of input or output file.
std::string UserInputPromptFile(std::string prompt)
{
        std::string file;
        std::cout << prompt << std::endl;
        std::cin >> file;
        return file;
}

// need to declare this for use in the following functions.
crc crcTable[256];

/* crcTableInit() creates the crc lookup table.
 * Only needs to be run once.
 */
void crcTableInit() {
    crc remainder;
    int dividend;
    unsigned char bit;

// calculate the remainder of all possible dividends
    for (dividend = 0; dividend < 256; ++dividend) {

        // start with dividend followed by zeroes
        remainder = dividend << (CRCBITS - 8);

        // division, bit by bit
        for (bit = 8; bit > 0; --bit) {
            if (remainder & CRCTOPBIT) { // current bit divides
                remainder = (remainder << 1) ^ POLYNOMIAL;
            } else {                     // current bit doesn't divide
                remainder = (remainder << 1);
            }
        }

        crcTable[dividend] = remainder;
    }
} /* crcTableInit() */


/* crcFun() calculates the crc value of a message and returns it. */
crc crcFun(char *message, int nBytes) {
    crc remainder = INITIAL_REMAINDER; // in case we get a packet which starts with a lot of zeroes
    int data; // index of the table (dividend)
    int byte; // which byte we are on currently

    // divide the message by the polynomial, one byte at a time.
    for (byte = 0; byte < nBytes; ++byte) {
        data = message[byte] ^ (remainder >> (CRCBITS - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }

// the remainder is the crc
    return remainder;
} /* crcFun() */

