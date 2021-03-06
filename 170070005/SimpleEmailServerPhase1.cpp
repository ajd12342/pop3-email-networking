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
	if(argc != 3){
		cerr << 
		"Usage: ./server <portNum> <passwdFile>"<<endl;
		return 1;
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

	int sockfdListen=socket(AF_INET,SOCK_STREAM,0);
	//char* ipAddr="127.0.0.1";
	struct sockaddr_in saddr;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(portNum);
	saddr.sin_addr.s_addr=INADDR_ANY;
	//inet_aton(ipAddr,&(saddr.sin_addr));
	for(int i=0;i<8;i++)
		saddr.sin_zero[i]='\0';

	//Allow reuse of address/port
	int reuse = 1;
	if (setsockopt(sockfdListen, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	#ifdef SO_REUSEPORT
		if (setsockopt(sockfdListen, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
			perror("setsockopt(SO_REUSEPORT) failed");
	#endif
		
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

	//Now, checks whether passwdFile can be opened
	string passwdFile=argv[2];
	ifstream ifs(passwdFile,ifstream::in);
	if(ifs.fail()){
		cerr<<
		"Failed to open/find file at "<<passwdFile
		<<endl;
		close(sockfdListen);
		return 3;
	}
	ifs.close();

	//Listen
	retval=listen(sockfdListen,MAX_BACKLOG);
	if(retval==-1){
		cerr<<
		"ListenFailed: "<<portNum<<endl;
		close(sockfdListen);
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
		close(sockfdListen);
		return 4;
	}
	int sockfd=retval;
	int cPort=caddr.sin_port;
	string cIP=inet_ntoa(caddr.sin_addr);
	cout<<"Client: "<<cIP<<":"<<cPort<<endl;

	//Start communication
	char* remMessage=new char[100];
	char* message=new char[100];
	int maxLen=100;
	bool changed=recvString(message,remMessage,maxLen,sockfd);
	delete[] remMessage;
	// Parse message
	if(changed){
	string msg=string(message);
	delete[] message;
	string user;
	string pass;
	bool parsed=true;
	int startFrom=0;
	int Userpos=msg.find("User: ",startFrom);
	if(Userpos!=0){
		parsed=false;
	}else{
		startFrom+=6;
		int Spacepos=msg.find(' ',startFrom);
		if(Spacepos==string::npos){
			parsed=false;
		}else{
			user=msg.substr(startFrom,
				Spacepos-startFrom);
			startFrom=(Spacepos+1);
			int Passpos=msg.find("Pass: ",startFrom);
			if(Passpos!=startFrom){
				parsed=false;
			}else{
				startFrom+=6;
				pass=msg.substr(startFrom,
					string::npos);
			}
		}
	}

	//Handle parsed/not-parsed
	bool loginDone=false;
	if(!parsed){
		cout<<"Unknown Command"<<endl;
	}else{
		vector<string> authUser;
		vector<string> authPass;
		string userit,passit;

		//Read from password file
		//Again check for existence of file(could have been removed while server was running)
		ifstream ifs(passwdFile,ifstream::in);
		if(ifs.fail()){
			cerr<<
			"Failed to open/find file at "<<passwdFile
			<<endl;
			close(sockfd);
			close(sockfdListen);
			return 3;
		}
		while(ifs>>userit>>passit){
			authUser.push_back(userit);
			authPass.push_back(passit);
		}
		ifs.close();

		//Search for user
		vector<string>::iterator findUser=find(authUser.begin(),authUser.end(),user);
		//User not found
		if(findUser==authUser.end()){
			cout<<"Invalid User"<<endl;
			close(sockfd);
			close(sockfdListen);
			return 0;
		//User found, but password wrong
		}else if(authPass[distance(authUser.begin(),findUser)]!=pass){
			cout<<"Wrong Passwd"<<endl;
			close(sockfd);
			close(sockfdListen);
			return 0;
		}else{
			//Authenticated user, welcome message sent
			string welcomeMessage=string("Welcome ")+user+string("\n");
			cout<<welcomeMessage;
			sendString(welcomeMessage,sockfd);
			loginDone=true;
		}
	}
	//Login done
	if(loginDone){
		//Try receiving command
		char* remMessage=new char[100];
		char* message=new char[100];
		int maxLen=100;
		bool changed=recvString(message,remMessage,maxLen,sockfd);
		delete[] remMessage;

		if(changed){
			string commandMsg=string(message);
			delete[] message;
			if(commandMsg=="quit"){
				cout<<"Bye "<<user<<endl;
			}
			else{
				//Unknown command received
				cout<<"Unknown command"<<endl;
			}
		}
	}
	}
	//Close
	close(sockfdListen);
	close(sockfd);
}