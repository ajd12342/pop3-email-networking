#include <iostream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
using namespace std;
int main(int argc, char* argv[]){

	//Checks for correct no. of arguments
	if(argc != 4){
		cerr << 
		"Usage: ./a.out <serverIPADDr:port> "<<
		"<username> <passwd>"<<endl;
		return 1;
	}

	//Parse input
	string delim=":";
	string ipp=argv[1];
	size_t next = ipp.find(delim,0);
	if(next==string::npos){
		cerr<<
		"Unable to parse IP Address and/or port"<<endl<<
		"Hence connection to server failed"<<endl;
		return 2;
	}
	const char* ipAddr=ipp.substr(0,next).c_str();
	string portStr=ipp.substr(next+1);
	int port;
	try{
		port=stoi(portStr);
	}catch(exception e){
		cerr<<
		"Unable to parse IP Address and/or port"<<endl<<
		"Hence connection to server failed"<<endl;
		return 2;
	}

	const char* username=argv[2];
	const char* passwd=argv[3];

	//Form dest addr
	struct sockaddr_in saddr;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(port);
	int retval;
	retval=inet_pton(AF_INET,ipAddr,&(saddr.sin_addr));
	if(retval==0){
		cerr<<
		"Unable to parse IP Address and/or port"<<endl<<
		"Hence connection to server failed"<<endl;
		return 2;
	}
	for(int i=0;i<8;i++)
		saddr.sin_zero[i]='\0';

	//Connect
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	retval=connect(sockfd,(struct sockaddr*) &saddr,
		sizeof(struct sockaddr));
	if(retval==-1){
		cerr<<
		"Connection to server "<<ipAddr<<":"<<
		port<<" failed"<<endl;
		return 2;
	}
	cout<<"ConnectDone: "<<ipAddr<<":"<<port<<endl;
	string messageStr=string("User: ")+
						string(username)+
						string(" Pass: ")+
						string(passwd);
	//const char* message=messageStr.c_str();
	char* message="hello! Hi";
	int messageRem=strlen(message)+1;
	cout<<message<<endl;
	cout<<"Message length: "<<messageRem<<endl;
	for(int i=0;i<messageRem;i++){
		cout<<(int)(message[i])<<endl;
	}
	while(messageRem!=0){
		int sentSize=send(sockfd,&message,
			messageRem,0);
		cout<<"Entered"<<endl;
		if(sentSize==-1){
			cerr<<"Unable to send credentials "<<
			"because a local error occurred. "<<
			"Retrying..."<<endl;
		}else{
			messageRem-=sentSize;
			//message+=sentSize;
		}
	}
	cout<<"Message Sent"<<endl;
	while(true){

	}
	//Close
	close(sockfd);
}