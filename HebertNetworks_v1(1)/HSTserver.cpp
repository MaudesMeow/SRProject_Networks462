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

#include <sys/ioctl.h>
#include <vector>
#include <chrono>

using namespace std;

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using namespace std::literals::chrono_literals;
using std::this_thread::sleep_for;

int portNumber;
int sock;
string fileName;
ofstream outFile;
int server_fd;
int new_socket;
int valread;
int numOfPackets = 0;
int bufferSize = 0;
int checkStatus;
string received;
char buf[0];
int n;

struct Packet {
        string packetNum;
        string sequenceNum;
        string checksum;
        string message;
};



void UserInputPromptPort()

{
	cout << "Enter port number: ";
        cin >> portNumber;
	
}



void UserInputPromptFile()

{
        cout << " What is the name of your file you would like to create (use with the extentsion .txt)?: ";
        cin >> fileName;
		ofstream outFile(fileName);	
}


void CreateSocket()
{
		sock = 0;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
		{
                perror("socket failed");
                exit(EXIT_FAILURE);
				
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
		{
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( portNumber );

        if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
			{
                perror("bind failed");
                exit(EXIT_FAILURE);
				
        }
        if (listen(server_fd, 3) < 0)
			{
                perror("listen");
                exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{
                perror("accept");
                exit(EXIT_FAILURE);
        }
}

int main(int argc, char const *argv[]) {
        
        UserInputPromptPort();

        UserInputPromptFile();
        
        
		CreateSocket();
        

        //start is the starting point of the clock as it is before starting communication
        time_point<Clock> start = Clock::now();
        //end is the point of the clock as it is every moment. Communication ends once enough
        //time has passed.
        time_point<Clock> end = Clock::now();
        auto diff = duration_cast<milliseconds>(end-start);

        //while the difference between the start time and the end time is < 10,000 milliseconds
        while(diff.count() < 10000) {
                diff = duration_cast<milliseconds>(end-start);
                end = Clock::now();
                received.clear();
                checkStatus = 0;
                memset(buf, 0, sizeof(buf));
                ioctl(new_socket, FIONREAD, &checkStatus);
                //if the socket is good,
                if(checkStatus > 0) {
                        //if we have recieved a packet, write it to the file, and print that we did it.
                        if(n = recv(new_socket, &bufferSize, sizeof(bufferSize), 0)) {
                                char buf[bufferSize];
                                n = recv(new_socket, buf, sizeof(buf), 0);
                                received.append(buf, buf+n);
                                if(received.length() != 0){
                                        numOfPackets++;
                                        cout << "Packet " << numOfPackets << " received: " << endl;
                                        outFile << received;
                                        start = Clock::now();
                                }
                        }
                }
        }
        string check = "md5sum " + fileName;
        outFile.close();
        system(check.c_str());
}

