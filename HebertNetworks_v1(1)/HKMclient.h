#ifndef HKMCLIENT_H
#define HKMCLIENT_H

#include <string>

std::string UserInputPromptAddr();
int UserInputPromptPacket();
int UserInputPromptWindow();
int UserInputPromptSequence();
int CreateSocketClient(int port, std::string ip);
int sendKillswitch(int sock);

// defining our packet struct to keep track of the timeouts for each one we send
typedef struct Packet{
public:
        int sequenceNum; // for reference, might not be needed. should be equal to the index of the srpBuffer array
        uint64_t timeLastSent; // used in timeout for each packet we send
        char *payload; // this is what gets sent to server
        }packet;


#endif