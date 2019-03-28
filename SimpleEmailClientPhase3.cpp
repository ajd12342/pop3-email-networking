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
	cout<<"argc"<<argc<<endl;
	//Checks for correct no. of arguments
	if(argc != 6){
		cerr << 
		"Usage: ./client <serverIPADDr:port> "<<
		"<username> <passwd> "<<
		"<local-folder> <list-of-messages>"<<endl;
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

	//Parse list of numbers
	char* listOfNosString=argv[5];
	vector<string> msgNumStrs;
	char* pch;
	pch=strtok(listOfNosString,",");
	while(pch!=NULL){
		msgNumStrs.push_back(string(pch));
		pch=strtok(NULL,",");
	}
	vector<int> msgNums(msgNumStrs.size());
	for(int i=0;i<msgNumStrs.size();i++){
		try{
			msgNums[i]=stoi(msgNumStrs[i]);
		}catch(exception e){
			cerr << 
			"Usage: ./client <serverIPADDr:port> "<<
			"<username> <passwd> "<<
			"<local-folder> <list-of-messages>"<<endl;
			close(sockfd);
			return 3;
		}
	}

	//Handle local-folder
	string localFolder=string(argv[4]);
	//Escape quotes inside localFolder later

	//Attempt to remove folder
	string remFolder=string("#/bin/bash\nrm -rf \"")
	+localFolder+string("\";");
	system(remFolder.c_str());

	//Create folder
	string makeFolder=string("#/bin/bash\n")+
	string("mkdir \"")+localFolder+string("\";");
	int status=system(makeFolder.c_str());
	if(status!=0){
		cerr<<"Failed to create "<<
		"directory at "<<localFolder<<endl;
		close(sockfd);
		return 4;
	}

	//Attempt accessing folder
	DIR* dir=opendir(localFolder.c_str());
	if(!dir){
		cerr<<
		"Failed to access directory at "<<
		localFolder<<endl;
		close(sockfd);
		return 4;
	}
	closedir(dir);



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

	//Send download request and receive files
	for(int reqNum: msgNums){
		//Send download request
		string req=to_string(reqNum);
		string reqMsg="RETRV "+req;
		sendString(reqMsg,sockfd);
		//Receive requested message

		//1.Receive filename
		char* remMessage=new char[100];
		char* filename=new char[100];
		int maxLen=100;
		//cout<<"Waiting for fname"<<endl;
		bool changed=recvString(filename,remMessage,maxLen,sockfd);
		delete[] remMessage;
		if(!changed){
			close(sockfd);
			return 0;
		}
		//cout<<filename<<endl;
		string filePathS=localFolder+"/"+filename;
		delete[] filename;
		const char* filePath=filePathS.c_str();
		FILE* fp=fopen(filePath,"wb");
		if(!fp){
			cerr<<"Unable to create message file"
			<<endl;
			close(sockfd);
			return 4;
		}

		//Receive filesize
		uint32_t* filesizePtr=new uint32_t;
		//cout<<"Waiting for fsize"<<endl;
		changed=recvInt(sockfd,filesizePtr);
		if(!changed){
			fclose(fp);
			close(sockfd);
			return 0;
		}
		//cout<<*filesizePtr<<endl;
		//Receive file
		if(*filesizePtr>0){
			pair<long,bool> retvals=recvAndStoreData(fp,1000,*filesizePtr,sockfd);
			if(!retvals.second){
				delete filesizePtr;
				fclose(fp);
				close(sockfd);
				return 0;
			}else{
				cout<<"Downloaded Message "<<req<<endl;
				if(retvals.first!=0){
					cout<<"Warning: Total filesize="<<*filesizePtr<<
					"B; Received filesize="<<(*filesizePtr-retvals.first)<<"B and connection closed."<<endl;
				}
			}
		}else{
			cout<<"Downloaded Message "<<req<<endl;
		}
		delete filesizePtr;
		fclose(fp);
	}

	// Send quit message
	sendString(string("quit"),sockfd);

	//Close
	close(sockfd);
}