#ifndef HKMSERVER_H
#define HKMSERVER_H

int CreateSocketServer(int port);

typedef struct Packet {
        bool isFull;
        char *message;
        int lengthOfPacket;
}packet;

#endif