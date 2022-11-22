#include <stdio.h>
#include <string>
#include <iostream>

#include "HKMcommon.hpp"

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

/* prompt user for how they want to generate the 'errorType' errors 
   returns 1 for randomly generated, 2 for user generated, and 0 for no errors
*/
int UserInputPromptErrorGenerationMethod(std::string errorType)
{
        int errorCode;
        std::cout << "How would you like to generate the" << errorType << " errors?" << std::endl;
        std::cout << "Randomly generated (Enter 1)." << std::endl;
        std::cout << "User generated (Enter 2)." << std::endl;
        std::cout << "Default: none (Enter any other value). " << std::endl;

        std::cin >> errorCode;

        switch (errorCode)
        {
        case 1:
                return 1;

        case 2: 
                return 2;
        
        default:
                return 0;
        }
}

/* prompt user for the number of errors to generate */
int UserInputPromptErrorCount(std::string errorType)
{
        int count;
        std::cout << "Enter the number of " << errorType << ": " << std::endl;
        std::cin >> count;
        return count;
}

/* prompt user for count numbers, and return them in an array */
int *UserInputPromptGenerateErrorArray(int count, std::string errorType)
{
        int *packNumsWithErrors = new int[count]();
        std::cout << "Enter the packet numbers for which " << errorType << ": " << std::endl;
        for (int i = 0; i < count; i++)
        {
                std::cin >> packNumsWithErrors[i];
        }

        return packNumsWithErrors;
        
}

/* generate random number between 5 and 200: */
int randomGeneratedErrorCount() 
{
        srand (time(NULL));
        int count = rand() % 200 + 5;
}

/* generate count random numbers between 0 and 2 million, and return them in an array */
int *randomGeneratedErrorArray(int count)
{
        int *packNumsWithErrors = new int[count]();
        srand(time(NULL)); // seeding rand()

        for (int i = 0; i < count; i++)
        {
                packNumsWithErrors[i] = rand() % 2000000; // mod 2 million
        }
        return packNumsWithErrors;
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