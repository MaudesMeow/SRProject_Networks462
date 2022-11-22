#ifndef HKMSERVER_HPP
#define HKMSERVER_HPP

int CreateSocketServer(int port);

typedef struct Packet {
        bool isFull;
        char *message;
        int lengthOfPacket;
}packet;

#endif