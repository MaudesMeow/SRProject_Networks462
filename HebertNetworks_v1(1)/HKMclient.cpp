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
#include <algorithm>
#include <chrono>

#include "HKMclient.hpp"
#include "HKMcommon.hpp"

using namespace std;

//prompts the user for the IP address we're going to use to send packets.
string UserInputPromptAddr()
{
        string ip;
        cout << "Enter IPaddress: " << endl;
        cin >> ip;
        return ip;	
}

//returns packet size from user input
int UserInputPromptPacket()
{
        int packet;
        cout << "Enter the packet size or -1 for default (" << PACKET_SIZE << "): " << endl;
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
        cout << "Enter the window size or -1 for default (" << WINDOW_SIZE << "): " << endl;
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
        cout << "Enter the sequence number size or -1 for default (" << SEQUENCE_SIZE << "): " << endl;
        cin >> sequence;
        if (sequence < 0)
        {
                sequence = SEQUENCE_SIZE;
        }
        
        return sequence;
}

int UserInputPromptTimeout()
{
        int timeout;
        cout << "Enter the timeout value or -1 for default (Generated from pinging the server)" << endl;
        cin >> timeout;
        return timeout;
}

auto generateTimeoutFromPing(string ip) 
{
        int pingCount = 10;
        chrono::high_resolution_clock::time_point pingStart;
        chrono::high_resolution_clock::time_point pingEnd;

        pingStart = chrono::high_resolution_clock::now();
        if (pingServer(ip, pingCount) != 0)
        {
                return chrono::nanoseconds(-1);
        }
        pingEnd = chrono::high_resolution_clock::now();

        auto pingDiff = pingEnd - pingStart; // time it took for pingCount pings

        return ((pingDiff / pingCount) * 32); // average time per ping, multiplied by 16 
}

// returns 0 for success, or something else for failure
int pingServer(string ip, int numPings)
{
        stringstream ss;
        string command;
        FILE *in;
        char buff[512];
        int exit_code;

        command = "ping -A -c " + to_string(numPings) + ip + " 2>&1";

        if(!(in = popen(command.c_str(), "r"))) // open process as read only
        {
                return -1;
        }

        while(fgets(buff, sizeof(buff), in) != NULL)    // put response into stream
        {
                ss << buff;
        }

        exit_code = pclose(in); // blocks until process is done; returns exit status of command

        return (exit_code == 0);
}

//creates a socket using a port number and an IP address. 
//returns the file descriptor for the socket if success, -1 if failure, and prints corresponding error message.
int CreateSocketClient(int port, string ip)
{
	int sock = 0, client_fd;
        struct sockaddr_in serv_addr;
        
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

void sendKillswitch(int sock) 
{
        // this tells the server we are done
        packet killswitch;
        int payloadSize = 7;
        int killcode = KILLCODE;

        killswitch.payload = new char[payloadSize + BYTES_OF_PADDING]();
        // putting the sequence number at the front of the packet
        killswitch.payload[0] = (char) ((killcode & 0xFF000000) >> 24); 
        killswitch.payload[1] = (char) ((killcode & 0x00FF0000) >> 16);
        killswitch.payload[2] = (char) ((killcode & 0x0000FF00) >> 8);
        killswitch.payload[3] = (char)  (killcode & 0x000000FF);

        killswitch.payload[sizeof(int)] = 'b';
        killswitch.payload[sizeof(int) + 1] = 'y';

        for (int i = 2; i < payloadSize; i++)
        {
                killswitch.payload[i + sizeof(int)] = 'e';
        }
        

        crc newcrc = crcFun(&killswitch.payload[0], payloadSize + sizeof(int));

        killswitch.payload[payloadSize + sizeof(int)]     = (char) ((newcrc & 0xFF000000) >> 24);
        killswitch.payload[payloadSize + sizeof(int) + 1] = (char) ((newcrc & 0x00FF0000) >> 16);
        killswitch.payload[payloadSize + sizeof(int) + 2] = (char) ((newcrc & 0x0000FF00) >> 8);
        killswitch.payload[payloadSize + sizeof(int) + 3] = (char)  (newcrc & 0x000000FF);

        int bufsize = payloadSize + BYTES_OF_PADDING;
        send(sock, &bufsize, sizeof(bufsize), 0);
        send(sock, killswitch.payload, bufsize, 0);
}



int main(int argc, char const *argv[]) {

        // initialize the crc table
        crcTableInit();

        // TESTING means we're using default values

        string IPaddress; // IP address of the server
        if (TESTING)
        {
                IPaddress = "10.35.195.219"; // IP for Poseidon0
        } else {
                IPaddress = UserInputPromptAddr();
        }
        
        int portNumber; // server port number to use
        if (TESTING)
        {
                portNumber = PORT_NUMBER;
        } else {
                portNumber = UserInputPromptPort();
        }
        
        string fileName; // name of the input file
        if (TESTING)
        {
                fileName = "HKMclient.cpp";
        } else {
                fileName = UserInputPromptFile("Enter the name of the input file: ");
        }
        
        int packetSize; // the max packet size
        if (TESTING)
        {
                packetSize = PACKET_SIZE;
        } else {
                packetSize = UserInputPromptPacket();
        }      

        int windowSize; // the size of our sliding window
        if (TESTING)
        {
                windowSize = WINDOW_SIZE;
        } else {
                windowSize = UserInputPromptWindow();
        }
        
        int sequenceNumSize; // the max sequence number value
        if (TESTING)
        {
                sequenceNumSize = SEQUENCE_SIZE;
        } else {
                sequenceNumSize = UserInputPromptSequence();
        }

        int *packetsToLose;
        int lostPacketCount;

        int lostGenerationMethod;
        if (TESTING)
        {
                lostGenerationMethod = 1;
        } else 
        {
                lostGenerationMethod = UserInputPromptErrorGenerationMethod("packet lost");
        }
        
        switch (lostGenerationMethod)
        {
                case 0: // no generated errors
                        lostPacketCount = 0;
                        break;
                case 1: // randomly generated errors
                        lostPacketCount = randomGeneratedErrorCount();
                        packetsToLose = randomGeneratedErrorArray(lostPacketCount);
                        break;
                case 2: // user generated errors
                        lostPacketCount = UserInputPromptErrorCount("packets to lose");
                        packetsToLose = UserInputPromptGenerateErrorArray(lostPacketCount, "packets to lose");
                        break;
                default: // didn't follow directions
                        return -1;         
        }

        sort(packetsToLose, packetsToLose+lostPacketCount); // our packets to lose array is sorted.

        int indexOfNextPacketToLose = 0;



        int *packetsToCorrupt;
        int corruptPacketCount;

        int corruptGenerationMethod;
        if (TESTING)
        {
                corruptGenerationMethod = 1;
        } else
        {
                corruptGenerationMethod = UserInputPromptErrorGenerationMethod("packet corrupted");
        }
        
        switch (corruptGenerationMethod)
        {
        case 0: // no generated errors
                corruptPacketCount = 0;
                break;
        case 1: // randomly generated errors
                corruptPacketCount = randomGeneratedErrorCount();
                packetsToCorrupt = randomGeneratedErrorArray(corruptPacketCount);
                break;
        case 2: // user generated errors
                corruptPacketCount = UserInputPromptErrorCount("packets to corrupt");
                packetsToCorrupt = UserInputPromptGenerateErrorArray(corruptPacketCount, "packets to corrupt");
                break;
        default: // something went wrong
                return -1;
        }
        
        sort(packetsToCorrupt, packetsToCorrupt+corruptPacketCount); // our packets to corrupt array is sorted.

        int indexOfNextPacketToCorrupt = 0;

        chrono::nanoseconds timeout; // time we allow to pass without receiving an ack before resending a packet
        int userTimeout;
        if (TESTING)
        {
                timeout = generateTimeoutFromPing(IPaddress);
        } else {
                userTimeout = UserInputPromptTimeout();
                if (userTimeout < 0)
                {
                        timeout = generateTimeoutFromPing(IPaddress);
                } else {
                        timeout = chrono::nanoseconds(userTimeout);
                }
        }

        if (timeout < chrono::nanoseconds(0))
        {
                cout << "timeout generation failed." << endl;
                return -1;
        }
        

        // bool *errorArray = (bool*)malloc(sizeof(bool) * (*sequenceSize));
        // errorArray = UserPromptErrorChoice(sequenceNumSize, *errorArray);

        // socket creation failed, exit the program (sockets are represented by integers)
        int sock;
        if ((sock = CreateSocketClient(portNumber, IPaddress)) < 0) {
                return -1;
        }

// send the packet size, window size and sequence number size to the server so they know what to use
        int *header = new int[3]();
        
        header[0] = packetSize;
        header[1] = windowSize;
        header[2] = sequenceNumSize;
        
        char ackHeaderRecv; // the value we 'catch' the ack in for the header

        // send the header
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
        int currentSequenceNum = 0; // the sequence number of the packet we are reading from the file. Always needs to be in the window. Might be the same as window upper bound?

        // defining our packet struct to keep track of the timeouts for each one we send
        typedef struct Packet{
        public:
                int packetBufSize; //used to resend packets
                int globalPacketNumber; //used to track packets. Global number for the simulation
                int sequenceNum; // for reference, might not be needed. should be equal to the index of the srpBuffer array
                chrono::high_resolution_clock::time_point timeoutTime; // used in timeout for each packet we send
                char *payload; // this is what gets sent to server
                bool isFull; //determines whether this packet is "zeroed", meaning we can write over it and ignore it.
                bool isAcked; //is this packed acked?
                }packet;

        // our selective repeat buffer to store the packtes in until they are acked
        packet srpBuffer[sequenceNumSize + 1];

        int windowUpperBound = windowSize;
        int windowLowerBound = 0; // these define our window on the client

        //while we haven't hit the end of the file, start sending packets.
        int globalPacketNumber = 0;
        while(!readStream.eof()){
                if((!((windowLowerBound < windowUpperBound) && (currentSequenceNum > windowUpperBound || currentSequenceNum < windowLowerBound))
                || (currentSequenceNum > windowUpperBound && currentSequenceNum < windowLowerBound))){

                        globalPacketNumber ++;
                        // create the next packet and add it to the sr buffer at the correct index
                        packet nextPacket;
                        cout << "lower bound: " << windowLowerBound << endl;
                        cout << "upper bound: " << windowUpperBound << endl;

                        // start with the sequence number we are on
                        nextPacket.sequenceNum = currentSequenceNum; // to use as reference later (if needed)
                        nextPacket.payload = new char[packetSize + BYTES_OF_PADDING]();

                        // putting the sequence number at the front of the packet
                        nextPacket.payload[0] = (char) ((currentSequenceNum & 0xFF000000) >> 24); 
                        nextPacket.payload[1] = (char) ((currentSequenceNum & 0x00FF0000) >> 16);
                        nextPacket.payload[2] = (char) ((currentSequenceNum & 0x0000FF00) >> 8);
                        nextPacket.payload[3] = (char)  (currentSequenceNum & 0x000000FF);
                        nextPacket.isFull = true; //this packet should be taken seriously, as it exists.
                        nextPacket.isAcked = false;
                        nextPacket.globalPacketNumber = globalPacketNumber;

                        int payloadSize = 0; // actual number of bytes read in from file

                        //pull characters from readStream until payload == one packet.
                        for (int i = 0; (i < packetSize) && readStream.get(placeHolder); i++){
                                nextPacket.payload[i+sizeof(currentSequenceNum)] = placeHolder;
                                payloadSize++;
                        }


                        //calculate the crc and add it to the end of the file
                        crc newcrc = crcFun(&nextPacket.payload[0], payloadSize + sizeof(currentSequenceNum));
                cout << "crc: " << newcrc << endl;
                        nextPacket.payload[payloadSize + sizeof(currentSequenceNum)]     = (char) ((newcrc & 0xFF000000) >> 24);
                        nextPacket.payload[payloadSize + sizeof(currentSequenceNum) + 1] = (char) ((newcrc & 0x00FF0000) >> 16);
                        nextPacket.payload[payloadSize + sizeof(currentSequenceNum) + 2] = (char) ((newcrc & 0x0000FF00) >> 8);
                        nextPacket.payload[payloadSize + sizeof(currentSequenceNum) + 3] = (char)  (newcrc & 0x000000FF);
                        
                        int bufsize = (payloadSize + BYTES_OF_PADDING); // size of packet to send
                        nextPacket.packetBufSize = bufsize;
                        //fail to send or send corrupt packets based on the global packet number
                       if ((lostPacketCount != 0) && (nextPacket.globalPacketNumber == packetsToLose[indexOfNextPacketToLose]))
                       {
                                cout << "losing packet " << nextPacket.globalPacketNumber << endl;
                                indexOfNextPacketToLose++;
                                if (indexOfNextPacketToLose == lostPacketCount)
                                {
                                        lostPacketCount = 0;
                                }
                       } else if ((corruptPacketCount !=0) && (nextPacket.globalPacketNumber == packetsToCorrupt[indexOfNextPacketToCorrupt]))
                       {

                                cout << "corrupting packet " << nextPacket.globalPacketNumber << endl;
                                packet tempPacket;
                                tempPacket.payload = new char[packetSize + BYTES_OF_PADDING]();

                                // copy the payload to a new packet so we can corrupt it when we send it but still save the uncorrupted version
                                for (int i = 0; i < packetSize + BYTES_OF_PADDING; i++)
                                {
                                        tempPacket.payload[i] = nextPacket.payload[i];
                                }
                                tempPacket.isAcked = false;
                                tempPacket.isFull = true;
                                
                                char temp = tempPacket.payload[0];
                                tempPacket.payload[0] = temp++; // makes sure this value gets changed
                                indexOfNextPacketToCorrupt++;

                                if (indexOfNextPacketToCorrupt == corruptPacketCount)
                                {
                                        corruptPacketCount = 0;
                                }

                                //if payload is bigger than just the sequence number and crc, we have a packet to send.
                                //send the packet and its size to the server, and tell the user we did it.
                                if(bufsize > BYTES_OF_PADDING){
                                        cout << "sending packet " << nextPacket.globalPacketNumber << endl;
                                        cout << "bytes sent: " << bufsize << endl;
                                        send(sock, &bufsize, sizeof(bufsize), 0);
                                        send(sock, tempPacket.payload, bufsize, 0); //sends the corrupted packet, but does timeout for the normal one.
                                        nextPacket.timeoutTime = chrono::high_resolution_clock::now() + timeout;
                                }
                       } else
                       {
                                //if payload is bigger than just the sequence number and crc, we have a packet to send.
                                //send the packet and its size to the server, and tell the user we did it.
                                if(bufsize > BYTES_OF_PADDING){
                                        cout << "sending packet " << nextPacket.globalPacketNumber << endl;
                                        cout << "bytes sent: " << bufsize << endl;
                                        send(sock, &bufsize, sizeof(bufsize), 0);
                                        send(sock, nextPacket.payload, bufsize, 0);
                                        //timeout variable from line 307
                                        nextPacket.timeoutTime = chrono::high_resolution_clock::now() + timeout;
                                }
                       }


                        //we're waiting on the ack for this now.
                        srpBuffer[currentSequenceNum] = nextPacket;
                        

                        // update the current sequence number
                        currentSequenceNum = (currentSequenceNum + 1) % (sequenceNumSize + 1);
                }

                int checkStatus;
                ioctl(sock, FIONREAD, &checkStatus);

                if (checkStatus)
                {
                
                        int ackReceived;
                        recv(sock, &ackReceived, sizeof(ackReceived), 0);

                        cout << "ack " << ackReceived << " received.\n\n" << endl;

                        //if we recieved an ack for the lower bound of our window, we need to make sure we aren't waiting on stuff anymore.
                        if(ackReceived == windowLowerBound){
                                
                                //clear this index. This packet "doesn't exist" as far as we are concerned.
                                srpBuffer[ackReceived].isFull = false;

                                //slide the window
                                windowLowerBound ++;
                                windowUpperBound ++;

                                //make sure we move the window to compensate for all other previously recieved acks for packets that we've recorded.
                                while(srpBuffer[windowLowerBound].isAcked && srpBuffer[windowLowerBound].isFull){

                                        srpBuffer[windowLowerBound].isFull = false;
                                        windowLowerBound ++;
                                        windowUpperBound ++;
                                        
                                }

                        } else {
                                //we've recieved the ack for this packet, but it isn't the lowerbound one.
                                srpBuffer[ackReceived].isAcked = true;
                        }
                }

                //check for timeouts, and resend all packets that have timed out.
                int index = windowLowerBound;
                auto timeNow = chrono::high_resolution_clock::now();
                while((index != windowUpperBound) && (index < globalPacketNumber)){
                        //if we timed out, resend packet from srpBuffer
                        if((srpBuffer[index].timeoutTime < timeNow) && !srpBuffer[index].isAcked && srpBuffer[index].isFull){
                                cout << "packet " << index << " timed out." << endl;
                                cout << "resending packet " << srpBuffer[index].globalPacketNumber << endl;

                                int resendBufsize = srpBuffer[index].packetBufSize;
                                cout << "bytes sent: " << resendBufsize << endl;

                                send(sock, &resendBufsize, sizeof(resendBufsize), 0);
                                send(sock, srpBuffer[index].payload, resendBufsize, 0);

                                srpBuffer[index].timeoutTime = timeNow + timeout; //timeout variable from line 307
                        }
                        index = (index +1) % (sequenceNumSize+1);
                }
                

        }

        int killAck;
        bool serverKilled = false;
        while (!serverKilled)
        {
                sendKillswitch(sock);
                int received = recv(sock, &killAck, sizeof(killAck), 0);

                if (received > 0)
                {
                        serverKilled = true;
                }
                

        }
        //we're done sending packets. finish everything up.
        cout << "Packets sent. Complete"<< endl;
        string verify = "md5sum " + fileName;
        system(verify.c_str());
                
}
