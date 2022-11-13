#ifndef HKMCLIENT_H
#define HKMCLIENT_H

#include <string>

/*
#define PORT_NUMBER 9021    // Default port number
#define WINDOW_SIZE 16      // Default window size
#define PACKET_SIZE 1024    // Default packet size
#define SEQUENCE_SIZE 64    // Default sequence number size
*/

std::string UserInputPromptAddr();
// int UserInputPromptPort();
// std::string UserInputPromptFile();
int UserInputPromptPacket();
int UserInputPromptWindow();
int UserInputPromptSequence();
int CreateSocketClient(int port, std::string ip);



#endif