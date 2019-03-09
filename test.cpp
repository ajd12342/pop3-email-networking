#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>

int main(){
	fd_set rfds;
	timeval tv;
	tv.tv_sec=2;
	tv.tv_usec=0;
	FD_ZERO(&rfds);
	FD_SET(0,&rfds);
	int retval= select(0+1,&rfds,NULL,NULL,&tv);
	if(retval==-1){
		perror("Error in select()");
	}else if(retval>0){
		printf("%s\n", "Data available");
	}else{
		printf("%s\n", "Timed out.");
	}
}