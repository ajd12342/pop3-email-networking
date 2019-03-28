#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#define MAX_BACKLOG 2
using namespace std;
//Send a given null-terminated string
void sendString(string messageStr,int sockfd){
	const char* c_message=messageStr.c_str();
	int len=100;//strlen(c_message)+1;
	char* message=new char[len];
	char* messageInit=message;
	strcpy(message,c_message);
	int messageRem=100;//strlen(message)+1;
	while(messageRem!=0){
		int sentSize=send(sockfd,message,
			messageRem,0);
		if(sentSize==-1){
			cerr<<"Unable to send "<<
			"because a local error occurred. "<<
			"Retrying..."<<endl;
		}else{
			messageRem-=sentSize;
			message=message+sentSize;
		}
	}
	delete[] messageInit;
}

//Send binary data
void sendData(FILE* file, long bufLen, long fileLen,int sockfd){
    unsigned char* buf=new unsigned char[bufLen];
    while(fileLen>0)
    {
    	fread(buf,sizeof(unsigned char),
    		min(bufLen,fileLen),file);

        long sentSize = send(sockfd,buf,min(bufLen,fileLen), 0);
        if (sentSize == -1)
        {
            cerr<<"Unable to send "<<
			"because a local error occurred. "<<
			"Retrying..."<<endl;
        }else{
        	fileLen-=sentSize;
    	}
    }
    delete[] buf;
}

//Send an integer
void sendInt(int sockfd,uint32_t number){
	uint32_t numNBO=htonl(number);
	char* buf=(char*) &numNBO;
	int remLen=sizeof(uint32_t);
	while(remLen>0){
		int sentSize=send(sockfd,buf,
			remLen,0);
		if(sentSize==-1){
			cerr<<"Unable to send "<<
			"because a local error occurred. "<<
			"Retrying..."<<endl;
		}else{
			remLen-=sentSize;
			buf+=sentSize;
		}
	}
}

//Receive an integer, return true on success
bool recvInt(int sockfd, uint32_t* numberPtr){
	char* buf=(char*) numberPtr;
	int recvLen=sizeof(uint32_t);
	while(recvLen>0){
		int recvSize=recv(sockfd,buf,recvLen,0);
		if(recvSize==-1){
			cerr<<
			"Error occurred while receiving"<<endl;
			exit(5);
		}
		if(recvSize==0) break;
		recvLen-=recvSize;
		buf+=recvSize;
	}
	*numberPtr = ntohl(*numberPtr);
	return recvLen==0;
}

//Receive a message till null character is hit, with trailing data placed in remMessage
bool recvString(char* message,char* remMessage,int maxLen, int sockfd){
	char* messageInit=message;
	bool changed=false;
	//Reads till null char is encountered

	//Receive till null/end of sending
	while(true){
		int recvSize=recv(sockfd,message,maxLen,0);
		if(recvSize==-1){
			cerr<<
			"Error occurred while receiving"<<endl;
			exit(5);
		}
		if(recvSize==0){
			break;
		}
		changed=true;
		
		//Checks for null char
		int i;
		for(i=0;(i<recvSize)
					&&message[i]!='\0';i++);
		if(i==recvSize){
			//Null char not found; Continue loop
			message+=recvSize;
			maxLen-=recvSize;
		}else{
			//If any characters were encountered
			//after '\0', store them in remMessage
			for(int j=i+1;j<recvSize;j++){
				remMessage[j-i-1]=message[j];
			}
			//Copy messageIter into message
			message+=recvSize;
			maxLen-=recvSize;
			break;
		}
	}
	message=messageInit;
	return changed;
}

//Receive partial data till connection close or bufLen received
pair<long,bool> recvAndStoreData(FILE* file, long bufLen, long fileLen, int sockfd){
	 unsigned char* fileBuf=new unsigned char[bufLen];
	 bool changed=false;
	//Receive till connection close/fileLen bytes are received
	while(fileLen>0){
		long recvSize=recv(sockfd,fileBuf,min(bufLen,fileLen),0);
		if(recvSize==-1){
			cerr<<
			"Error occurred while receiving"<<endl;
			exit(5);
		}
		if(recvSize==0){
			break;
		}
		changed=true;
		fwrite(fileBuf,sizeof(unsigned char),recvSize,
			file);
		fileLen-=recvSize;
	}
	delete[] fileBuf;
	return make_pair(fileLen,changed);
}
//Receive message and output it
bool recvMessageAndOutput(int sockfd,int maxLen){
	char* remMessage=new char[maxLen];
	char* message=new char[maxLen];
	bool changed=recvString(message,remMessage,maxLen,sockfd);
	if(changed){
		cout<<message;
	}
	delete[] remMessage;
	delete[] message;
	return changed;
} 
//Find all subdirectories and files given a DIR*
vector<string> subDirFiles(DIR* dir){
	vector<string> dirfiles;
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		string name=string(ent->d_name);
		if(name=="."|| name==".."){

		}else{
			dirfiles.push_back(string(ent->d_name));
		}
	}
	return dirfiles;
}

int main(int argc, char* argv[]){

	//Checks for correct no. of arguments
	if(argc != 4){
		cerr << 
		"Usage: ./client <serverIPADDr:port> "<<
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
	bool changed=recvMessageAndOutput(sockfd,100);
	if(!changed){
		close(sockfd);
		return 0;
	}

	// Send quit message
	sendString(string("quit"),sockfd);

	//Close
	close(sockfd);
}