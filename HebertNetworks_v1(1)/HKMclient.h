#ifndef HKMCLIENT_H
#define HKMCLIENT_H

#include <string>

std::string UserInputPromptAddr();
int UserInputPromptPacket();
int UserInputPromptWindow();
int UserInputPromptSequence();
int CreateSocketClient(int port, std::string ip);



#endif