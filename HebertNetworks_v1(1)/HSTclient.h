#include <string>

#define PORT_NUMBER 9021    // Default port number
#define WINDOW_SIZE 25      // Default window size
#define PACKET_SIZE 1000    // Default packet size


std::string UserInputPromptAddr();
int UserInputPromptPort();
std::string UserInputPromptFile();
int UserInputPromptPacket();
int UserInputPromptWindow();
int CreateSocket(int port, std::string ip);
