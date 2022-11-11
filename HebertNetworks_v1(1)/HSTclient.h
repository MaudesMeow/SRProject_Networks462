#include <string>

#define PORT_NUMBER 9021    // Default port number

std::string UserInputPromptAddr();
int UserInputPromptPort();
std::string UserInputPromptFile();
int UserInputPromptPacket();
int CreateSocket(int port, std::string ip);
