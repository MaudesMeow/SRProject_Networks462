#include <string.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/ioctl.h>
#include <cstdio>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include "HKMclient.h"
#include "HKMcommon.h"

using namespace std;

//prompts the user for the IP address we're going to use to send packets.
string UserInputPromptAddr()
{
        string ip;
        cout << "Enter IPaddress: ";
        cin >> ip;
        return ip;	
}

//returns packet size from user input
int UserInputPromptPacket()
{
        int packet;
        cout << "Enter the packet size or -1 for default (" << PACKET_SIZE << "): ";
        cin >> packet;
        if (packet < 0)
        {
                packet = PACKET_SIZE;
        }
        
        return packet;
}

//returns window size from user input
int UserInputPromptWindow()
{
        int window;
        cout << "Enter the window size or -1 for default (" << WINDOW_SIZE << "): ";
        cin >> window;
        if (window < 0)
        {
                window = WINDOW_SIZE;
        }
        
        return window;
}

//returns sequence number size from user input
int UserInputPromptSequence()
{
        int sequence;
        cout << "Enter the sequence number size or -1 for default (" << SEQUENCE_SIZE << "): ";
        cin >> sequence;
        if (sequence < 0)
        {
                sequence = SEQUENCE_SIZE;
        }
        
        return sequence;
}

//creates a socket using a port number and an IP address. 
//returns the file descriptor for the socket if success, -1 if failure, and prints corresponding error message.
int CreateSocketClient(int port, string ip)
{
	int sock = 0, valueRead, client_fd;

        struct sockaddr_in serv_addr;
        char buffer[1024] = {0};
        
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) 
                {
                printf("\n Socket creation error \n");
                return -1;
                }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0){
                printf("\n Invalid address \n");
                return -1;
        }
        if((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
                printf("\nConnection Failed \n");
                return -1;
        }
	return sock;
}


int main(int argc, char const *argv[]) {

        //creates socket, gets file and packet size
        string IPaddress;
        if (TESTING)
        {
                IPaddress = "10.35.195.219";
        } else {
                IPaddress = UserInputPromptAddr();
        }
        
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
                fileName = "HKMclient.h";
        } else {
                fileName = UserInputPromptFile("Enter the name of the input file: ");
        }
        
        int packetSize;
        if (TESTING)
        {
                packetSize = PACKET_SIZE;
        } else {
                packetSize = UserInputPromptPacket();
        }      

        int windowSize;
        if (TESTING)
        {
                windowSize = WINDOW_SIZE;
        } else {
                windowSize = UserInputPromptWindow();
        }
        
        int sequenceNumSize;
        if (TESTING)
        {
                sequenceNumSize = SEQUENCE_SIZE;
        } else {
                sequenceNumSize = UserInputPromptSequence();
        }
        
        // socket creation failed, exit the program (sockets are represented by integers)
        int sock;
        if ((sock = CreateSocketClient(portNumber, IPaddress)) < 0) {
                return -1;
        }

// send the packet size and window size to the server so they know what to use
        int checkStatus = 0;
        int *header = new int[3]();
        
        header[0] = packetSize;
        header[1] = windowSize;
        header[2] = sequenceNumSize;
        
        bool ackRecv = false;
        char ackHeaderRecv;// = new int[1]();

        send(sock, header, HEADER_SIZE, 0);

        // server sends back 1 for successful reception of header information        
        recv(sock, &ackHeaderRecv, sizeof(ackHeaderRecv), 0);
        if (ackHeaderRecv != '1')
        {
                printf("\nHeader ack not received\n");
                return -1;
        }
        
        //creates an ifstream named "readStream" that reads from the user's selected file
        ifstream readStream(fileName);
        char placeHolder;        
        string fileInput =""; // this is what we send to the server
        int currentSequenceNum = 0; // the sequence number of the packet we are reading from the file. Always needs to be in the window

        // defining our packet struct to keep track of the timeouts for each one we send
        typedef struct Packet{
        public:
                int sequenceNum; // for reference, might not be needed
                uint64_t timeLastSent; // used in timeout for each packet we send
                string payload = ""; // this is what gets sent to server
                }packet;

        // our selective repeat buffer to store the packtes in until they are acked
        packet srpBuffer[sequenceNumSize + 1];

        int leftWindowEdge, rightWindowEdge; // these define our window on the client

        //while we haven't hit the end of the file, start sending packets.
        while(!readStream.eof()){
                
                // create the next packet and add it to the sr buffer at the correct index
                packet nextPacket;
                srpBuffer[currentSequenceNum] = nextPacket;

                // start with the sequence number we are on
                nextPacket.sequenceNum = currentSequenceNum; // to use as reference later (if needed)
                nextPacket.payload = to_string(currentSequenceNum); // putting the sequence number at the front of the packet

                //pull characters from readStream until payload == one packet.
                for (int i = 0; i < packetSize && readStream.get(placeHolder); i++){
                        nextPacket.payload.push_back(placeHolder);
                }

                //calculate the crc and add it to the end of the file
                crc newcrc = crcFun(nextPacket.payload.data(), nextPacket.payload.size());
                nextPacket.payload += to_string(newcrc);



                //if payload is bigger than just the sequence number and crc, we have a packet to send.
                //send the packet and its size to the server, and tell the user we did it.
                if(!nextPacket.payload.size() > BYTES_OF_PADDING){
                        int bufsize = nextPacket.payload.size();
                        send(sock, &bufsize, sizeof(bufsize), 0);
                        send(sock, nextPacket.payload.data(), nextPacket.payload.size(), 0);
                }

        }

        //we're done sending packets. finish everything up.
        cout << "Packets sent. Complete"<< endl;
        string verify = "md5sum " + fileName;
        system(verify.c_str());
                
}