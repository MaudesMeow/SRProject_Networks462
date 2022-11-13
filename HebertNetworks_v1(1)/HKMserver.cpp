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
//using namespace std::literals::chrono_literals;
using std::this_thread::sleep_for;

//int port;
//int sock;
//string fileName;
//because you can't actually return these, this is nessecary.
//ofstream outFile;
//int server_fd;
//int new_socket;
//int valread;
//int numOfPackets = 0;
//int bufferSize = 0;
//int checkStatus;
//string received;
//char buffer[0];
//int n;

struct Packet {
        string packetNum;
        string sequenceNum;
        string checksum;
        string message;
};

/*
//prompts user for port number, and returns user input
int UserInputPromptPort(){
        int port;
	cout << "Enter port number: ";
        cin >> port;
        return port;
	
}



void UserInputPromptFile() {

        cout << " What is the name of your file you would like to create (use with the extentsion .txt)?: ";
        cin >> fileName;
	ofstream outFile(fileName);	
}
*/

// creates a listening socket for the client to connect to. 
// returns the file descriptor for the socket if successfull, -1 if failure
int CreateSocketServer(int port){
	int sock = 0;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        int server_fd;

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{
                perror("socket failed");
                return -1;	
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{
                perror("setsockopt failed");
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

        int new_socket = -1;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
	{
                perror("accept failed");
                return -1;
        }

        return new_socket;
}

int main(int argc, char const *argv[]) {
        
        int portNumber = UserInputPromptPort();
        string fileName = UserInputPromptFile("Enter the name of the file to which you would like to output: ");
	int sock = CreateSocketServer(portNumber);

        //start is the starting point of the clock as it is before starting communication
        time_point<Clock> start = Clock::now();
        //end is the point of the clock as it is every moment. Communication ends once enough
        //time has passed.
        time_point<Clock> end = Clock::now();
        auto diff = duration_cast<milliseconds>(end-start);
    
        //bool gotHeader = false;
        
        int headerStatus = 0;
        int packetSize;
        int windowSize;
        int sequenceNumSize;
        int *headerRecv = {0};

        while (headerRecv[0] <=0)
        {        
                // ioctl makes sure there is information to read. Stores bytes to read in checkStatus
                ioctl(sock, FIONREAD, &headerStatus);
                        
                if (headerStatus > 0)
                {
                        // client sends information on bufferSize, windowSize, and sequencNumSize
                        recv(sock, headerRecv, sizeof(headerRecv), 0);
                        packetSize = headerRecv[0];
                        windowSize = headerRecv[1];
                        sequenceNumSize = headerRecv[2];
                }
        }

        // send ack for header here
        int ack = 1;
        send(sock, &ack, sizeof(ack), 0);
        
        // //recieve header and send ack (****Don't think this is needed anymore****)
        // while(!gotHeader){}
               
        //         //if we've recieved a packet
        //         if(n = recv(new_socket, &bufferSize, sizeof(bufferSize), 0)) {
        //                 char buffer[bufferSize];
        //                 n = recv(new_socket, buffer, sizeof(buffer), 0);
        //                         received.append(buffer, buffer+n);
        //                         if(received.length() != 0){
        //                                 //ideas on how to read: recv() or use read() with an ifstream
        //                                 //honestly, IDK which one of these works, or even how to get our information from them.
        //                         }

        //         }

        // }

        string received = "";
        int n;
        char *buffer;
        ofstream outFile;
        int numOfPackets = 0;
        int bufferSize = 0;
        
        //while the difference between the start time and the end time is < 10,000 milliseconds
        while(diff.count() < 10000) {
                diff = duration_cast<milliseconds>(end-start);
                end = Clock::now();
                received.clear();
                int checkStatus = 0;
                memset(buffer, 0, sizeof(buffer)); //allocates memory for buffer. Debatable if we need to do this at all.
                ioctl(sock, FIONREAD, &checkStatus); //used to check if the socket has information to be recieved
                //if the socket is good,
                if(checkStatus > 0) {
                        //recv is a funciton that checks if the socket has recieved something from the client.
                        if(n = recv(sock, &bufferSize, sizeof(bufferSize), 0)) {
                                char buffer[bufferSize];
                                n = recv(sock, buffer, sizeof(buffer), 0);
                                received.append(buffer, buffer+n);
                                if(received.length() != 0){
                                        numOfPackets++;
                                        //prints to console which packet was recieved.
                                        cout << "Packet " << numOfPackets << " received: " << endl;
                                        //write to the outfile
                                        outFile << received;
                                        //reset the start time so that the server won't timeout.
                                        start = Clock::now();
                                }
                        }
                }
        }
        string check = "md5sum " + fileName;
        outFile.close();
        std::system(check.c_str());
}

