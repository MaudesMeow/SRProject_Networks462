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


int main(int argc, char const *argv[]) {

        // initialize the crc table
        crcTableInit();

        //creates socket, gets file and packet size
        string IPaddress;
        if (TESTING)
        {
                IPaddress = "10.35.195.219"; // IP for Poseidon0
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

        int windowSize;
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
                int sequenceNum; // for reference, might not be needed. should be equal to the index of the srpBuffer array
                uint64_t timeLastSent; // used in timeout for each packet we send
                char *payload; // this is what gets sent to server
                }packet;

        // our selective repeat buffer to store the packtes in until they are acked
        packet srpBuffer[sequenceNumSize + 1];

        int windowUpperBound, windowLowerBound; // these define our window on the client

        //while we haven't hit the end of the file, start sending packets.
        while(!readStream.eof()){

                // create the next packet and add it to the sr buffer at the correct index
                packet nextPacket;
                srpBuffer[currentSequenceNum] = nextPacket;

                // start with the sequence number we are on
                nextPacket.sequenceNum = currentSequenceNum; // to use as reference later (if needed)
                nextPacket.payload = new char[packetSize + BYTES_OF_PADDING]();

                // putting the sequence number at the front of the packet
                nextPacket.payload[0] = (char) ((currentSequenceNum & 0xFF000000) >> 24); 
                nextPacket.payload[1] = (char) ((currentSequenceNum & 0x00FF0000) >> 16);
                nextPacket.payload[2] = (char) ((currentSequenceNum & 0x0000FF00) >> 8);
                nextPacket.payload[3] = (char)  (currentSequenceNum & 0x000000FF);

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

                //if payload is bigger than just the sequence number and crc, we have a packet to send.
                //send the packet and its size to the server, and tell the user we did it.
                if(bufsize > BYTES_OF_PADDING){
        cout << "bytes sent: " << bufsize << endl;
                        send(sock, &bufsize, sizeof(bufsize), 0);
                        send(sock, nextPacket.payload, bufsize, 0);
                }

// receiving our ack here.
// TODO: implement this shiz.
                int ackReceived;
                recv(sock, &ackReceived, sizeof(ackReceived), 0);

                // update the sequence number
                currentSequenceNum = (currentSequenceNum + 1) % (sequenceNumSize + 1);

        }

        //we're done sending packets. finish everything up.
        cout << "Packets sent. Complete"<< endl;
        string verify = "md5sum " + fileName;
        system(verify.c_str());
                
}

bool* RandomlyGeneratedErrors(int sequenceSize){
        //allocate memory for new bool array length of sequence
        bool *errors = new bool[sequenceSize](); 
        //provides a new ~seed~ to return a different random value
        srand(time(NULL));

        for(int ii =0; ii < sequenceSize; ii++){
                //calling a random value between 1-100 and if the value returned is less than 10, an error will be put into that index number
                if ((rand() % 100 + 1 ) <= 10) {
                        errors[ii] = true;
                }



        }
        //returns our modified array with errors, from here we can see if the ack or packet being sent is at the same index as an error within the errors array. 
        return errors; 

}


bool* UserInputErrors(int sequenceSize) {





        string userInput;
        
        string inputIndexValue;;
        //allocating memory for bool array 
        bool *errors = new bool[sequenceSize](); 
        cout << "At what values should an error occur? (type in values seperated with one space)";
        //using getline method to store user input into string userInput
        getline(cin, userInput);
        //string stream in, able to seperate user input by space
        stringstream ssin(userInput);
        
        //while there are still numbers add each to correct place in errors array
        while (ssin >> inputIndexValue){
                
                // stoi == string object integer (I think), converts string to integer value
                errors[stoi(inputIndexValue)] = true;


        }

        return errors;

}


bool UserPromptErrorChoice(int* sequenceSize, bool** errorArray){

        
        string userInput = "";

        cout << "How would you like to generate errors?" << endl;
        cout << "Default: none (hit enter). " << endl;
        cout << "Randomly generated (press 1)." << endl;
        cout << "User generated (press 2)." << endl;

        
        getline(cin, userInput);
        
        if (userInput.empty()) {
                userInput = -1; 
        }

        if (!userInput.empty()) {
                while (*errorArray == NULL) {
                        if (userInput.compare("-1") == 0){
                                *errorArray = (bool*)malloc(sizeof(bool) * (*sequenceSize)); 
                                for(int i = 0; i < *sequenceSize; i++){
                                        (*errorArray)[i] = false;
                                }
                        } else if(userInput.compare("1") == 0){
                                *errorArray = RandomlyGeneratedErrors(*sequenceSize);
                        } else if(userInput.compare("2") == 0){
                                *errorArray = UserInputErrors(*sequenceSize);
                        } else{
                                cout << "Invalid input. Try again." << endl;
                                getline(cin, userInput);
                                }


                        }

                }
                return errorArray;


}
