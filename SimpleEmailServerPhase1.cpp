#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <fstream>
#include <stdlib.h>
using namespace std;
int main(int argc, char* argv[]){

	//Checks for correct no. of arguments
	if(argc != 3){
		cerr << 
		"Usage: ./a.out <portNum> <passwdFile>"<<endl;
		return 1;
	}

	//If correct usage, attempts to bind.
	//Binds to IP address INADDR_ANY
	int portNum;
	try{
		portNum=stoi(argv[1]);
	}catch(exception e){
		cerr<<
		"Given port "<<argv[1]<< 
		" is not a number"<<endl;
		return 2;
	}
	string passwdFile=argv[2];
	const char* ipaddr="127.0.0.1";

	int sockfd=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in saddr;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(portNum);
	inet_aton(ipaddr,&(saddr.sin_addr));
	for(int i=0;i<8;i++)
		saddr.sin_zero[i]='\0';
	int bindRet=bind(sockfd,(struct sockaddr*)&saddr,
		sizeof(struct sockaddr));

	//Checks for bind failure
	if(bindRet==-1){
		cerr<<
		"Bind failed on port "<<portNum<<endl;
		return 2;
	}
	//Now, checks whether passwdFile can be opened
	ifstream ifs(passwdFile,ifstream::in);
	if(ifs.fail()){
		cerr<<
		"Failed to open/find file at "<<passwdFile
		<<endl;
		return 3;
	}
	//Everything worked till here
}