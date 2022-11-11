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
int windowSize;
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


string UserInputPromptAddr()
{
        string ip;
        cout << "Enter IPaddress: ";
        cin >> ip;
        return ip;	
}

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


string UserInputPromptFile()
{
        string file;
        cout << "Enter the name of your file: ";
        cin >> file;
        return file;
}

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

        string IPaddress = UserInputPromptAddr();
	int portNumber = UserInputPromptPort();
        string fileName = UserInputPromptFile();
        int packetSize = UserInputPromptPacket();
        int windowSize = UserInputPromptWindow();
        
        // socket creation failed, exit the program
        if (CreateSocket(portNumber, IPaddress) < 0) {
                return 0;
        }

        int header[2] = {packetSize, windowSize};



        pFile = fopen(fileName.c_str(), "r");
        ifstream is(fileName);
        char placeHolder;
        ostringstream temp;
        string fileInput;

        while(!is.eof()){
                fileInput.clear();
                //payOut.clear();
                for (int i = 0; i < packetSize && is.get(placeHolder); i++){
                        fileInput.push_back(placeHolder);
                }
                if(!fileInput.empty()){
                        //payOut.append(fileInput);
                        nextSequenceNum++;
                        int bufsize = fileInput.length();
                        send(sock, &bufsize, sizeof(bufsize), 0);
                        send(sock, fileInput.data(), fileInput.size(), 0);
                        cout << "Packet " << nextSequenceNum << " sent: " << endl;
                }

        }
        cout << "Packets sent. Complete"<< endl;
        string verify = "md5sum " + fileName;
        system(verify.c_str());

}