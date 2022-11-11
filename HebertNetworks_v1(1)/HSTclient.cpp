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
#include "HSTclient.h"

using namespace std;

//int portNumber;
//string IPaddress;
int sock;
int valueRead; // what is this variable?
//int packetSize;
//int windowSize;
//int seqrange;
int nextSequenceNum = 0;
//string line;
//string reset;
//string fileName;
//string payOut = "";
//string fileInput;
FILE * pFile;

string empty = "\n";


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
        cout << " What is the name of your file?: ";
        cin >> file;
        return file;
}

//returns packet size from user input
int UserInputPromptPacket()
{
        int packet;
        cout << "What is the packet size?: ";
        cin >> packet;
        return packet;
}

//creates a socket using a port number and an IP address. 
//returns 0 if success, -1 if failure, and prints corrisponding error message.
int CreateSocket(int port, string ip)
{
	
	sock = 0, valueRead;

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
		return 0;
}



int main(int argc, char const *argv[]) {
        int portNumber;
        string IPaddress = "";

        //creates socket, gets file and packet size
        IPaddress = UserInputPromptAddr();
	portNumber = UserInputPromptPort();
        CreateSocket(portNumber, IPaddress);
        string fileName = UserInputPromptFile();
        int packetSize = UserInputPromptPacket();

        //creates a file to print to?
        pFile = fopen(fileName.c_str(), "r");
        //creates an ifstream named "is" that reads from the user's selected file
        ifstream is(fileName);//consider changing name of the ifstream so that it isn't called "is"
        char placeHolder;
        ostringstream temp;
        string fileInput;

        //while we haven't hit the end of the file, start sending packets.
        while(!is.eof()){

                fileInput.clear();
                //payOut.clear();

                //pull characters from is until fileInput == one packet.
                for (int i = 0; i < packetSize && is.get(placeHolder); i++){
                        fileInput.push_back(placeHolder);
                }

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

}