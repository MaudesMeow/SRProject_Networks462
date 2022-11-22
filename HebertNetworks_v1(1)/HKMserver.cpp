#include <cstring>
#include <sys/types.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <poll.h>
#include <algorithm>
#include <sys/ioctl.h>
#include <vector>
#include <chrono>

#include "HKMserver.h"
#include "HKMcommon.h"

using namespace std;

// creates a listening socket for the client to connect to. 
// returns the file descriptor for the socket if successfull, -1 if failure
int CreateSocketServer(int port){
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int server_fd;

        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) 
	{
                perror("socket failed");
                return -1;	
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( port );

        if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
	{
                perror("bind failed");
                return -1;
        }

        if (listen(server_fd, 3) < 0)
	{
                perror("listen failed");
                return -1;
        }

        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket<0) 
	{
                perror("accept failed");
                return -1;
        }

        return new_socket;
}

int main(int argc, char const *argv[]) {
        
        // initialize the crc table
        crcTableInit();

        int portNumber;
        if (TESTING)
        {
                portNumber = PORT_NUMBER;
        } else {
                portNumber = UserInputPromptPort();
        }
        
        string fileName;
        if (TESTING)
        {
                fileName = "output.txt";
        } else {
                fileName = UserInputPromptFile("Enter the name of the file to which you would like to output: ");
        }

        int *acksToLose; // packet numbers of the acks we are losing
        int ackCount; // number of acks to lose


        switch (UserInputPromptErrorGenerationMethod("ack lost"))
        {
        case 0: // no generated errors
                ackCount = 0;
                acksToLose = new int[0]();
                break;
        
        case 1: // randomly generated errors
                ackCount = randomGeneratedErrorCount();
                acksToLose = randomGeneratedErrorArray(ackCount);
                break;

        case 2: // user generated errors
                ackCount = UserInputPromptErrorCount("acks to lose");
                acksToLose = UserInputPromptGenerateErrorArray(ackCount, "acks to lose");
                break;
        default: // try again, sucka
                return -1;
        }

        sort(acksToLose, acksToLose+ackCount); // our acks to lose array is sorted.
                

	int sock = CreateSocketServer(portNumber);

        
        int *headerRecv = new int[3]();

        // client sends information on bufferSize, windowSize, and sequencNumSize
        recv(sock, headerRecv, HEADER_SIZE, 0);
        int packetSize = headerRecv[0];
        int windowSize = headerRecv[1];
        int sequenceNumSize = headerRecv[2];

        // send ack for header here
        char ack = '1';
        send(sock, &ack, sizeof(ack), 0);

        
        ofstream outFile;
        outFile.open(fileName, ios::out | ios::app);
        int numOfPackets = 0;
        int bufferSize = 0;
        int windowLowerBound = 0;        //sequence number used by selective repeat algorithm (basically the left side of our window)
        int windowUpperBound = windowSize;

        bool weAreDone = false;
        
        // the index of the next ack we need to not send. makes checking easier.
        int indexOfNextAckToLose = 0;
        
        //creates selectiverepeatbuffer
        packet *selectiveRepeatBuffer;
        selectiveRepeatBuffer = new packet[sequenceNumSize + 1](); // want largest index to be sequenceNumSize

        while (!weAreDone)
        {
                int checkStatus = 0;

                ioctl(sock, FIONREAD, &checkStatus); //used to check if the socket is working.
                //if the socket is good,
                if(checkStatus > 0) {
                        // recv is a funciton that reads from the socket and returns the number of bytes it read.
                        // do we need to send two messages here? Can we assume a set amount is sent each time? (packetSize + BYTES_OF_PADDING)
                        
                        int n = recv(sock, &bufferSize, sizeof(bufferSize), 0); // number of bytes received
                        if(n > 0) {
                                char buffer[bufferSize];
                                int pktlen = recv(sock, buffer, sizeof(buffer), 0);
                                cout << "\n\npktlen: " << pktlen << endl;


                                //this is the packet that we're going to pull from the buffer.
                                packet newPacket;
                                newPacket.message = new char[pktlen]();
                                newPacket.isFull = true;
                                newPacket.lengthOfPacket = pktlen;

                                //creates a packet by pulling characters from buffer array up to the packetsize
                                for(int i = 0; i < pktlen; i++){
                                        newPacket.message[i] = buffer[i];
                                }
                                
                                bool passedChecksum = false;
                                                                
                                crc crcFromClient = (((((unsigned int) newPacket.message[pktlen-4]) << 24) & 0xFF000000) | 
                                                     ((((unsigned int) newPacket.message[pktlen-3]) << 16) & 0x00FF0000) |
                                                     ((((unsigned int) newPacket.message[pktlen-2]) << 8)  & 0x0000FF00) |
                                                      (((unsigned int) newPacket.message[pktlen-1])        & 0x000000FF));
                                cout << "crcFromClient: " << crcFromClient << endl;
                                
                                crc crcCalculated = crcFun(&newPacket.message[0], pktlen - sizeof(crc)); // don't include the crc at the end of the packet when we are calculating it.
                                cout << "crcCalculated: " << crcCalculated << endl;
                                
                                if (crcFromClient == crcCalculated)
                                {
                                        passedChecksum = true;
                                }

                                if(passedChecksum){

                                        //get the sequence number of the packet that we've recieved. 
                                        //this equation converts the first 4 chars (1 Byte each) into an int (4 Bytes)
                                        int packetSequenceNumber = (((((unsigned int) newPacket.message[0]) << 24) & 0xFF000000) | 
                                                                    ((((unsigned int) newPacket.message[1]) << 16) & 0x00FF0000) |
                                                                    ((((unsigned int) newPacket.message[2]) << 8)  & 0x0000FF00) |
                                                                     (((unsigned int) newPacket.message[3])        & 0x000000FF));
                                        
                                        // the client will send a sequence number of -1 with dummy message when it is done reading in from the file.
                                        if (packetSequenceNumber == -1)
                                        {
                                                weAreDone = true;
                                                continue; 
                                        }
                                        
                                        
                                        //above windowUpperBound or below windowLowerBound
                                        if((windowLowerBound < windowUpperBound) && (packetSequenceNumber > windowUpperBound || packetSequenceNumber < windowLowerBound)){
                                                
                                                if ((ackCount != 0) && (packetSequenceNumber == acksToLose[indexOfNextAckToLose]))
                                                {
                                                        indexOfNextAckToLose++;
                                                        if (indexOfNextAckToLose == ackCount) // it's the last ack to lose
                                                        {
                                                                ackCount = 0; // this ensures we won't drop any more acks (on purpose)
                                                        }                                                        
                                                } else {
                                                        //send an ack to the client indicating that we have recieved the packet.
                                                        send(sock, &packetSequenceNumber, sizeof(packetSequenceNumber), 0);
                                                }

                                        }//between windowupperbound and windowLowerBound
                                        else if(packetSequenceNumber > windowUpperBound && packetSequenceNumber < windowLowerBound){
                                                if ((ackCount != 0) && (packetSequenceNumber == acksToLose[indexOfNextAckToLose]))
                                                {
                                                        indexOfNextAckToLose++;
                                                        if (indexOfNextAckToLose == ackCount) // it's the last ack to lose
                                                        {
                                                                ackCount = 0; // this ensures we won't drop any more acks (on purpose)
                                                        }                                                        
                                                } else {
                                                        //send an ack to the client indicating that we have recieved the packet.
                                                        send(sock, &packetSequenceNumber, sizeof(packetSequenceNumber), 0);
                                                }
                                        }
                                        else{

                                                numOfPackets++;
                                                //prints to console which packet was recieved.
                                                cout << "Packet " << numOfPackets << " received: " << endl;

                                                // we need to not send this ack because it matches the next one in our array
                                                if ((ackCount != 0) && (packetSequenceNumber == acksToLose[indexOfNextAckToLose])) 
                                                {
                                                        indexOfNextAckToLose++;
                                                        if (indexOfNextAckToLose == ackCount) // it's the last ack to lose
                                                        {
                                                                ackCount = 0; // this ensures we won't drop any more acks (on purpose)
                                                        }                                                        
                                                } else {
                                                        //send an ack to the client indicating that we have recieved the packet.
                                                        send(sock, &packetSequenceNumber, sizeof(packetSequenceNumber), 0);
                                                }

                                                selectiveRepeatBuffer[packetSequenceNumber] = newPacket;
                                            
                                                //write packets to our output file
                                                while(selectiveRepeatBuffer[windowLowerBound].isFull){
                                                        //

                                                        for(int j = sizeof(packetSequenceNumber); j < (selectiveRepeatBuffer[windowLowerBound].lengthOfPacket - (int) sizeof(crc)); j++){
                                                                outFile << selectiveRepeatBuffer[windowLowerBound].message[j] << flush;
                                                        }
                                                        
                                                        //remove current packet from selectiveRepeatBuffer
                                                        selectiveRepeatBuffer[windowLowerBound].isFull = false;
                                                        
                                                        //move the sliding window after effectively writing to the output file
                                                        windowLowerBound = (windowLowerBound+1) % (sequenceNumSize+1);
                                                        windowUpperBound = (windowUpperBound+1) % (sequenceNumSize+1);
                                                }

                                        } // checking if in window

                                } // passed checksum

                        } // if n > 0

                } // checkstatus (ioctl confirmed)

       } // while loop

        delete[] selectiveRepeatBuffer;

        string check = "md5sum " + fileName;
        outFile.close();
        std::system(check.c_str());

} // main()

