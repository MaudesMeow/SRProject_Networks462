

#include <stdio.h>
#include <string>
#include <iostream>
#include "HKMcommon.h"


//prompts te user for the port number
int UserInputPromptPort()
{
        int port;
        std::cout << "Enter port number or -1 for default (" << PORT_NUMBER << "): ";
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
        std::cout << prompt;
        std::cin >> file;
        return file;
}