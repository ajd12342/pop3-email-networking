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
		"Usage: ./a.out <portNum> <passwdFile>"<<
		" <user-database>"<<endl;
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
	ifs.close();

	//Check if user-database exists
	char* dirFile=argv[3]; 
	DIR* dir=opendir(dirFile);
	if(!dir){
		cerr<<
		"Failed to open/find directory at "<<
		dirFile<<endl;
		return 4;
	}
	closedir(dir);

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
		return 5;
	}
	cout<<"ListenDone: "<<portNum<<endl;

	//Loop to handle clients sequentially
	while(true){
	//Accept
	struct sockaddr_in caddr;
	unsigned int addrlen=sizeof(struct sockaddr);
	retval=accept(sockfdListen,(struct sockaddr*)&caddr,
		&addrlen);
	if(retval==-1){
		cerr<<
		"AcceptFailed"<<endl;
		return 5;
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
		//User found, but password wrong
		}else if(authPass[distance(authUser.begin(),findUser)]!=pass){
			cout<<"Wrong Passwd"<<endl;
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
		//Try receiving LIST
		char* remMessage=new char[100];
		char* message=new char[100];
		int maxLen=100;
		bool changed=recvString(message,remMessage,maxLen,sockfd);
		delete[] remMessage;

		if(changed){
			string listMsg=string(message);
			delete[] message;

			if(listMsg=="LIST"){
				//LIST received

				//Check if user-database exists, again 
				DIR* dir=opendir(dirFile);
				if(!dir){
					cerr<<
					"Failed to open/find directory at "<<
					dirFile<<endl;
					close(sockfd);
					close(sockfdListen);
					return 4;
				}

				//Traverse directory
				vector<string> directories=
				subDirFiles(dir);
				closedir(dir);
  				vector<string>::iterator it=
  				find(directories.begin(),
  					directories.end(),user);

  				//Check whether we found user's dir
  				if(it==directories.end()){
  					cout<<user<<": Folder Read Fail\n";
  				}else{
  					//Found user dir
  					string userDirS=string(dirFile)+
  					"/"+user;
  					const char* userDir=userDirS.c_str();
  					DIR* dir=opendir(userDir);
  					//Check if userDir can be opened
					if(!dir){
						cout<<user<<": Folder Read Fail\n";
					}else{
						//Check number of messages
						vector<string> files=
						subDirFiles(dir);
						int noOfFiles=files.size();
						string infoMsg=user+
						string(": No of messages ")+
						to_string(noOfFiles)+string(" \n");
						cout<<infoMsg;
						sendString(infoMsg,sockfd);

						//Receive quit message/closed connection
						char* remMessage=new char[100];
						char* message=new char[100];
						int maxLen=100;
						bool changed=recvString(message,remMessage,maxLen,sockfd);
						delete[] remMessage;
						if(changed){
							string quitMsg=string(message);
							delete[] message;
							if(quitMsg=="quit"){
								cout<<"Bye "<<user<<endl;
							}else{
								cout<<"Unknown command"<<endl;
							}
						}
					}
  				}
			}else{
				//Unknown command received
				cout<<"Unknown command"<<endl;
			}
		}
	}

	}
	//Close client socket
	close(sockfd);
	}
	
	//Close listening socket	
	close(sockfdListen);
}