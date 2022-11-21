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
#include "HKMserver.h"
#include "HKMcommon.h"

#include <sys/ioctl.h>
#include <vector>
#include <chrono>

using namespace std;

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

struct Packet {
        string packetNum;
        string sequenceNum;
        string checksum;
        string message;
};


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


	int sock = CreateSocketServer(portNumber);

        // //start is the starting point of the clock as it is before starting communication
        // time_point<Clock> start = Clock::now();
        // //end is the point of the clock as it is every moment. Communication ends once enough
        // //time has passed.
        // time_point<Clock> end = Clock::now();
        // auto diff = duration_cast<milliseconds>(end-start);
            
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
        int currentSequenceNumber = 0;        //sequence number used by selective repeat algorithm
        int windowLowerBound = 0;
        int windowUpperBound = windowSize;
        //many c++ operations don't accept chars as parameters.

// these are both ints already, is this necessary?
        int packetSizeInt = packetSize; 
        int sequenceNumSizeInt = sequenceNumSize;

        int checkStatus;

        // TODO: We need to put a loop here to keep reading. Not sure this diff one is the best way to go about it?
        while (true)
        {
        
        // //while the difference between the start time and the end time is < 10,000 milliseconds
        // while(diff.count() < 10000) {
                // diff = duration_cast<milliseconds>(end-start);
                // end = Clock::now();
                checkStatus = 0;
                // 

                ioctl(sock, FIONREAD, &checkStatus); //used to check if the socket is working.
                //if the socket is good,
                if(checkStatus > 0) {
                        // recv is a funciton that reads from the socket and returns the number of bytes it read.
                        // do we need to send two messages here? Can we assume a set amount is sent each time? (packetSize + BYTES_OF_PADDING)
                        
                        int n = recv(sock, &bufferSize, sizeof(bufferSize), 0); // number of bytes received
                        if(n > 0) {
                                char buffer[bufferSize];
                                int pktlen = recv(sock, buffer, sizeof(buffer), 0);
                        cout << "pktlen: " << pktlen << endl;
                                //creates selectiverepeatbuffer
                                char** selectiveRepeatBuffer;
                                selectiveRepeatBuffer = new char*[sequenceNumSizeInt + 1](); // want largest index to be sequenceNumSize

                                for(int i = 0; i < sequenceNumSizeInt; i++){

                                        selectiveRepeatBuffer[i] = new char[packetSizeInt]();

                                }
                               
                                //auto[sequenceNumSizeInt][packetSizeInt] selectiveRepeatBuffer;
                                //auto[packetSizeInt] packet; 
                                
                                //this is the packet that we're going to pull from the buffer.
                                char* packet;
                                packet = new char[pktlen]();


                                //creates a packet by pulling characters from buffer array up to the packetsize
                                for(int i = 0; i < pktlen; i++){

                                        packet[i] = buffer[i];

                                }
                                
                                bool passedChecksum = false;
                                                                
                                crc crcFromClient = (((((unsigned int) packet[pktlen-4]) << 24) & 0xFF000000) | 
                                                     ((((unsigned int) packet[pktlen-3]) << 16) & 0x00FF0000) |
                                                     ((((unsigned int) packet[pktlen-2]) << 8)  & 0x0000FF00) |
                                                      (((unsigned int) packet[pktlen-1])        & 0x000000FF));
                cout << "crcFromClient: " << crcFromClient << endl;
                                
                                crc crcCalculated = crcFun(&packet[0], pktlen - sizeof(crc)); // don't include the crc at the end of the packet when we are calculating it.
                cout << "crcCalculated: " << crcCalculated << endl;
                                
                                if (crcFromClient == crcCalculated)
                                {
                                        passedChecksum = true;
                                }
                                

                                if(passedChecksum){

                                        //get the sequence number of the packet that we've recieved. 
                                        //this equation converts the first 4 chars (1 Byte each) into an int (4 Bytes)
                                        int packetSequenceNumber = ((packet[0] << 24) + (packet[1] << 16) + (packet[2] << 8) + packet[3]);
                cout << "packetSequenceNumber: " << packetSequenceNumber << endl;
                                        
                                        int packetSequenceInt = packetSequenceNumber - 0;

                                        //above windowUpperBound or below windowLowerBound
                                        if((windowLowerBound < windowUpperBound) && (packetSequenceInt > windowUpperBound || packetSequenceInt < windowLowerBound)){

                                                //send an ack to the client indicating that we have recieved the packet.
                                                send(sock, &packetSequenceNumber, sizeof(packetSequenceNumber), 0);

                                        }//between windowupperbound and windowLowerBound
                                        else if(packetSequenceInt > windowUpperBound && packetSequenceInt < windowLowerBound){

                                                //send an ack to the client indicating that we have recieved the packet.
                                                send(sock, &packetSequenceNumber, sizeof(packetSequenceNumber), 0);
                                        }
                                        else{

                                                numOfPackets++;
                                                //prints to console which packet was recieved.
                                                cout << "Packet " << numOfPackets << " received: " << endl;

                                                //send an ack to the client indicating that we have recieved the packet.
                                                send(sock, &packetSequenceNumber, sizeof(packetSequenceNumber), 0);

                                                //deep copy the packet onto the selectiveRepeatBuffer
                                                for(int j = 0; j < packetSizeInt; j++){

                                                        // skip the sequence number when copying it in, and don't include the crc at the end
                                                        selectiveRepeatBuffer[packetSequenceInt][j] = packet[j + sizeof(packetSequenceNumber)];

                                                }

                                                //write packets to our output file
                                                while(selectiveRepeatBuffer[currentSequenceNumber]){
                                                        //
                                                        for(int j = 0; j < packetSizeInt; j++){
                                                                outFile << selectiveRepeatBuffer[currentSequenceNumber][j];
                                                        }
                                                        //remove current packet from selectiveRepeatBuffer
                                                        selectiveRepeatBuffer[currentSequenceNumber] = 0;
                                                        //update currentSequenceNumber. Remember, it wraps around.
                                                        currentSequenceNumber = currentSequenceNumber+1 %sequenceNumSize+1;
                                                        //move the sliding window after effectively writing to the output file
                                                        windowLowerBound = windowLowerBound+1 %sequenceNumSize+1;
                                                        windowUpperBound = windowUpperBound+1 %sequenceNumSize+1;
                                                }
                                               // start = Clock::now();

                                        }

                                }

                                /*
                                //writes all packets
                                if(received.length() != 0){
                                        //write to the outfile
                                        outFile << received;
                                        //reset the start time so that the server won't timeout.
                                        start = Clock::now();
                                }

                                */

                                 //delete memory I allocated
                                for(int i = 0; i < sequenceNumSizeInt; i++){

                                        delete[] selectiveRepeatBuffer[i];

                                }
                                delete[] selectiveRepeatBuffer;

                                delete[] packet;

                        }
                }
       }
        string check = "md5sum " + fileName;
        outFile.close();
        std::system(check.c_str());
}

