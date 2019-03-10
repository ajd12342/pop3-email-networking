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
		" is not a number"<<endl<<
		"Hence bind failed on port "<<argv[1]<<endl;;
		return 2;
	}
	const char* ipaddr="127.0.0.1";

	int sockfdListen=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in saddr;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(portNum);
	inet_pton(AF_INET,ipaddr,&(saddr.sin_addr));
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

	//Start communication
	int recvSize=0;
	char* messageIter=new char[100];
	char* messageIterInit=messageIter;
	char* remMessage=new char[100];
	char* message=new char[100];
	int maxLen=100;
	//Reads till null char is encountered
	int iter=0; //Rem later
	while(true){
		recvSize=recv(sockfd,messageIter,maxLen,0);

		iter++;
		cout<<"Recvd "<<iter<<"times"<<endl;
		cout<<"Recvd size: "<<recvSize<<endl;
		if(recvSize==-1){
			cerr<<
			"Error occurred while receiving"<<endl;
			return 5;
		}
		if(recvSize==0){
			cout<<
			"Transmission completed gracefully"<<endl;
			break;
		}
		//Checks for null char
		int i;
		for(i=0;(i<recvSize)
					&&messageIter[i]!='\0';i++);
		if(i==recvSize){
			//Null char not found; Continue loop
			cout<<"Null Not found"<<endl;
			messageIter+=recvSize;
			maxLen-=recvSize;
		}else{
			//If any characters were encountered
			//after '\0', store them in remMessage
			for(int j=i+1;j<recvSize;j++){
				remMessage[j-i-1]=messageIter[j];
			}
			//Copy messageIter into message
			char* ptr=messageIterInit;
			char* messagePtr=message;
			while(ptr!=(messageIter+i+1)){
				*messagePtr=*ptr;
				ptr+=1;
				messagePtr+=1;
			}
			cout<<"Null found at "<<i<<endl;
			messageIter+=recvSize;
			maxLen-=recvSize;
			break;
		}
	}
	//delete[] messageIterInit;
	printf("%p\n",message);
	int i=0;
	while(
		*message!='\0'
		){
		i++;
		cout<<*message;
		message+=1;
	}
	cout<<i<<endl;
	cout<<"Outside"<<endl;
	while(true){

	}
	//Close
	close(sockfd);
	close(sockfdListen);
}