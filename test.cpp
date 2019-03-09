#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
using namespace std;
int main(){
	const char* a="AAA";
	const char* b="BBB";
	const char* c="dss";
	cout<<(int)c[0]<<' '<<(int)c[1]<<endl;
}