#ifndef HKMCLIENT_HPP
#define HKMCLIENT_HPP

#include <string>
#include <chrono>

std::string UserInputPromptAddr();
int UserInputPromptPacket();
int UserInputPromptWindow();
int UserInputPromptSequence();
int CreateSocketClient(int port, std::string ip);
void sendKillswitch(int sock);
int UserInputPromptTimeout();
auto generateTimeoutFromPing(std::string ip);
int pingServer(std::string ip, int numPings);

// defining our packet struct to keep track of the timeouts for each one we send
        typedef struct Packet{
        public:
                int packetBufSize; //used to resend packets
                int globalPacketNumber; //used to track packets. Global number for the simulation
                int sequenceNum; // for reference, might not be needed. should be equal to the index of the srpBuffer array
                std::chrono::high_resolution_clock::time_point timeoutTime; // used in timeout for each packet we send
                char *payload; // this is what gets sent to server
                bool isFull; //determines whether this packet is "zeroed", meaning we can write over it and ignore it.
                bool isAcked; //is this packed acked?
                }packet;

#endif