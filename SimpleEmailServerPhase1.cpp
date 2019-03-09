#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#define MAX_BACKLOG 2
using namespace std;
int main(int argc, char* argv[]){

	//Checks for correct no. of arguments
	if(argc != 3){
		cerr << 
		"Usage: ./a.out <portNum> <passwdFile>"<<endl;
		return 1;
	}

	//Now, checks whether passwdFile can be opened
	string passwdFile=argv[2];
	ifstream ifs(passwdFile,ifstream::in);
	if(ifs.fail()){
		cerr<<
		"Failed to open/find file at "<<passwdFile
		<<endl;
		return 3;
	}

	//Bind
	int portNum;
	try{
		portNum=stoi(argv[1]);
	}catch(exception e){
		cerr<<
		"Given port "<<argv[1]<< 
		" is not a number"<<endl;
		return 2;
	}
	const char* ipaddr="127.0.0.1";

	int sockfdListen=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in saddr;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(portNum);
	inet_aton(ipaddr,&(saddr.sin_addr));
	for(int i=0;i<8;i++)
		saddr.sin_zero[i]='\0';
	int retval;
	retval=bind(sockfdListen,(struct sockaddr*)&saddr,
		sizeof(struct sockaddr));

	//Checks for bind failure
	if(retval==-1){
		cerr<<
		"Bind failed on port "<<portNum<<endl;
		return 2;
	}
	cout<<"BindDone: "<<portNum<<endl;

	//Listen
	retval=listen(sockfdListen,MAX_BACKLOG);
	if(retval==-1){
		cerr<<
		"ListenFailed: "<<portNum<<endl;
		return 4;
	}
	cout<<"ListenDone: "<<portNum<<endl;

	//Accept
	struct sockaddr_in caddr;
	unsigned int addrlen=sizeof(struct sockaddr);
	retval=accept(sockfdListen,(struct sockaddr*)&caddr,
		&addrlen);
	if(retval==-1){
		cerr<<
		"AcceptFailed"<<endl;
		return 4;
	}
	int sockfd=retval;
	int cPort=caddr.sin_port;
	string cIP=inet_ntoa(caddr.sin_addr);
	cout<<"Client: "<<cIP<<":"<<cPort<<endl;

	
	//Close
	close(sockfd);
	close(sockfdListen);
}