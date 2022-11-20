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

//int portNumber;
//string IPaddress;
//int sock;               
int valueRead;          // what is this variable?
//int packetSize;
//int windowSize;
//int seqrange;
//int nextSequenceNum = 0;
//string line;
//string reset;
//string fileName;
//string payOut = "";
//string fileInput;
//FILE * pFile;
//string empty = "\n";


typedef struct Packet{
        public:
                uint32_t packetID;
                uint32_t packetSeqNum;
                string inside;
}pktype;

//prompts the user for the IP address we're going to use to send packets.
string UserInputPromptAddr()
{
        string ip;
        cout << "Enter IPaddress: ";
        cin >> ip;
        return ip;	
}

/*
//prompts te user for the port number
int UserInputPromptPort()
{
        int port;
        cout << "Enter port number or -1 for default (" << PORT_NUMBER << "): ";
        cin >> port;
        if (port < 0)
        {
                port = PORT_NUMBER;
        }
        
        return port;
}
//returns name of input file.
string UserInputPromptFile()
{
        string file;
        cout << "Enter the name of your file: ";
        cin >> file;
        return file;
}
*/

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





//creates a socket using a port number and an IP address. 

//returns the file descriptor for the socket if success, -1 if failure, and prints corresponding error message.
int CreateSocketClient(int port, string ip)
{
	int sock = 0, valueRead;

        struct sockaddr_in serv_addr;
        char buffer[1024] = {0};
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
                {
                printf("\n Socket creation error \n");
                return -1;
                }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_port = htons(port);

        if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0){
                printf("\n Invalid address \n");
                return -1;
        }
        if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                printf("\nConnection Failed \n");
                return -1;
        }
		return sock;
}


int main(int argc, char const *argv[]) {

        bool* errorArray = NULL;
        UserPromptErrorChoice(5, errorArray);

        //creates socket, gets file and packet size
        string IPaddress = UserInputPromptAddr();
	int portNumber = UserInputPromptPort();
        string fileName = UserInputPromptFile("Enter the name of the input file: ");
        int packetSize = UserInputPromptPacket();
        int windowSize = UserInputPromptWindow();
        int sequenceNumSize = UserInputPromptSequence();

        
        // socket creation failed, exit the program (sockets are represented by integers)
        int sock;
        if (sock = CreateSocketClient(portNumber, IPaddress) < 0) {
                return 0;
        }

// send the packet size and window size to the server so they know what to use
        int checkStatus = 0;
        int *header = new int[3]();
        
        header[0] = packetSize;
        header[1] = windowSize;
        header[2] = sequenceNumSize;
        //int *ackHeaderRecv = {0};

        pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLIN;
        int rc;
        int timeout = -1; // still need to implement this, but -1 means it blocks until the event occurs
        bool ackRecv = false;
        int *ackHeaderRecv = new int[1]();

        while (ackHeaderRecv[0] != 1) // while we haven't gotten a successful ack from server
        {
        
                send(sock, &header, sizeof(header), 0);

                rc = poll(&pfd, 1, timeout);

                if (rc < 0)
                {
                        printf("poll error");
                        exit(1);
                }

                if (rc == 0)
                {
                        printf("poll timed out");
                        exit(0);
                }

                // only get here if poll() found something to read from the socket
                // server sends back 1 for successful reception of header information
                recv(sock, ackHeaderRecv, sizeof(ackHeaderRecv), 0);
        }


/* old method for sending header and receiving ack
        while (ackHeaderRecv[0] <= 0) 
        {
                while (ackHeaderRecv[0] == 0) 
                {
                send(sock, &header, sizeof(header), 0);
                // ioctl makes sure there is information to read. Stores bytes to read in checkStatus
                ioctl(sock, FIONREAD, &checkStatus);
                int *ackHeaderRecv = {0};
                if (checkStatus > 0)
                {
                // server sends back 1 for successful reception of header information
                        recv(sock, ackHeaderRecv, sizeof(ackHeaderRecv), 0);
                        // if ackHeaderRecv returns -1, print an invalid connection & close ( I think this would go here?)
                        if (ackHeaderRecv[0] == -1) 
                        {
                        perror("invalid connection");
                        exit(1);
                        }
                // TODO: 
                //        Timeout implementation here at some point
                //        have server receive this header and send the ack 
                }
                }
                
                
        
        }
        //send header here *************
*/
        
        // the server got the header, so we are good to continue sending the file.
        
                //creates a file to print to?
                FILE *pFile = fopen(fileName.c_str(), "r");
                //creates an ifstream named "readStream" that reads from the user's selected file
                ifstream readStream(fileName);
                char placeHolder;
                ostringstream temp;
                string fileInput;

                //while we haven't hit the end of the file, start sending packets.
                while(!readStream.eof()){

                        fileInput.clear();
                        //payOut.clear();

                        //pull characters from readStream until fileInput == one packet.
                        for (int i = 0; i < packetSize && readStream.get(placeHolder); i++){
                                fileInput.push_back(placeHolder);
                        }

                        int nextSequenceNum = 0;
                        //if fileInput isn't empty, we have a packet to send.
                        //send the packet and its size to the server, and tell the user we did it.
                        if(!fileInput.empty()){
                                //payOut.append(fileInput);
                                nextSequenceNum++;
                                int bufsize = fileInput.length();
                                send(sock, &bufsize, sizeof(bufsize), 0);
                                send(sock, fileInput.data(), fileInput.size(), 0);
                                cout << "Packet " << nextSequenceNum << " sent: " << endl;
                        }

                }

                //we're done sending packets. finish everything up.
                cout << "Packets sent. Complete"<< endl;
                string verify = "md5sum " + fileName;
                system(verify.c_str());
        
        // server never acked the header, so we don't know that the connection is solid.
        /*else {
                cout << "Couldn't verify header info with server, exiting now." << endl;
                return 0;
        }*/
        
        

        
        

}