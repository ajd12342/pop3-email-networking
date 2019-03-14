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
//Send a given null-terminated string
void sendString(string messageStr,int sockfd){
	const char* c_message=messageStr.c_str();
	int len=strlen(c_message)+1;
	char* message=new char[len];
	char* messageInit=message;
	strcpy(message,c_message);
	int messageRem=strlen(message)+1;
	// cout<<message<<endl;
	// cout<<"Message length: "<<messageRem<<endl;
	while(messageRem!=0){
		int sentSize=send(sockfd,message,
			messageRem,0);
		// cout<<sentSize;
		// cout<<"Entered"<<endl;
		if(sentSize==-1){
			cerr<<"Unable to send "<<
			"because a local error occurred. "<<
			"Retrying..."<<endl;
		}else{
			messageRem-=sentSize;
			message=message+sentSize;
		}
	}
	// cout<<"Message Sent"<<endl;
	delete[] messageInit;
}

//Receive a message till null character is hit, with trailing data placed in remMessage
bool recvString(char* message,char* remMessage,int maxLen, int sockfd){
	char* messageIter=new char[maxLen];
	char* messageIterInit=messageIter;
	bool changed=false;
	//Reads till null char is encountered
	// int iter=0; //Rem later

	//Receive till null/end of sending
	while(true){
		int recvSize=recv(sockfd,messageIter,maxLen,0);

		// iter++;
		// cout<<"Recvd "<<iter<<"times"<<endl;
		// cout<<"Recvd size: "<<recvSize<<endl;
		if(recvSize==-1){
			cerr<<
			"Error occurred while receiving"<<endl;
			exit(5);
		}
		if(recvSize==0){
			break;
		}
		//Checks for null char
		int i;
		for(i=0;(i<recvSize)
					&&messageIter[i]!='\0';i++);
		if(i==recvSize){
			//Null char not found; Continue loop
			messageIter+=recvSize;
			maxLen-=recvSize;
		}else{
			//If any characters were encountered
			//after '\0', store them in remMessage
			for(int j=i+1;j<recvSize;j++){
				remMessage[j-i-1]=messageIter[j];
			}
			//Copy messageIter into message
			changed=true;
			char* ptr=messageIterInit;
			char* messagePtr=message;
			while(ptr!=(messageIter+i+1)){
				*messagePtr=*ptr;
				ptr+=1;
				messagePtr+=1;
			}
			//cout<<"Null found at "<<i<<endl;
			messageIter+=recvSize;
			maxLen-=recvSize;
			break;
		}
	}
	delete[] messageIterInit;
	return changed;
}
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
	retval=inet_aton(ipAddr,&(saddr.sin_addr));
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

	//Send Message
	sendString(string("User: ")+
						string(username)+
						string(" Pass: ")+
						string(passwd),sockfd);
	
	//Check for message from server/closed connection
	char* remMessage=new char[100];
	char* message=new char[100];
	int maxLen=100;
	bool changed=recvString(message,remMessage,maxLen,sockfd);
	if(changed){
		cout<<message;
	}
	delete[] remMessage;
	delete[] message;
	// Send quit message
	sendString(string("quit"),sockfd);

	//Close
	close(sockfd);
}