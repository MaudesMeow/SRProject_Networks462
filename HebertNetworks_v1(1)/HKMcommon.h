#ifndef HKMCOMMON_H
#define HKMCOMMON_H

#include <stdio.h>
#include <string>

#define PORT_NUMBER 9021    // Default port number
#define WINDOW_SIZE 16      // Default window size
#define PACKET_SIZE 1024    // Default packet size
#define SEQUENCE_SIZE 64    // Default sequence number size

int UserInputPromptPort();
std::string UserInputPromptFile(std::string prompt);


#endif