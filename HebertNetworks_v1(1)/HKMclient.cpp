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

using namespace std;

int portNumber;
string IPaddress;
int sock;
int valueRead;
int packetSize;
//int windowSize;
//int seqrange;
int nextSequenceNum = 0;
//string line;
//string reset;
string fileName;
string payOut = "";
string fileInput;
FILE * pFile;

string empty = "\n";


typedef struct Packet{
        public:
                uint32_t packetID;
                uint32_t packetSeqNum;
                string inside;
}pktype;


void UserInputPromptAddr()

{
	cout << "Enter port number: ";
        cin >> portNumber;

        cout << "Enter IPaddress: ";
        cin >> IPaddress;
	
}


void UserInputPromptFile()

{
	cout << "What is the packet size?: ";
        cin >> packetSize;

        cout << " What is the name of your file?: ";
        cin >> fileName;
	
}

int CreateSocket()
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
      serv_addr.sin_port = htons(portNumber);
      serv_addr.sin_port = htons(portNumber);

        if(inet_pton(AF_INET, IPaddress.c_str(), &serv_addr.sin_addr)<=0){
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
        portNumber =9000;
        IPaddress = "";

        UserInputPromptAddr();
		CreateSocket();
        
		UserInputPromptFile();


        pFile = fopen(fileName.c_str(), "r");
        ifstream is(fileName);
        char placeHolder;
        ostringstream temp;

        while(!is.eof()){
                fileInput.clear();
                payOut.clear();
                for (int i = 0; i < packetSize && is.get(placeHolder); i++){
                        fileInput.push_back(placeHolder);
                }
                if(!fileInput.empty()){
                        payOut.append(fileInput);
                        nextSequenceNum++;
                        int bufsize = payOut.length();
                        send(sock, &bufsize, sizeof(bufsize), 0);
                        send(sock, payOut.data(), payOut.size(), 0);
                        cout << "Packet " << nextSequenceNum << " sent: " << endl;
                }

        }
        cout << "Packets sent. Complete"<< endl;
        string verify = "md5sum " + fileName;
        system(verify.c_str());

}