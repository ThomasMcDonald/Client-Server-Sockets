#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

int connect_server(char * address, char * port);



int main(int argc, char  *argv[])
{
	connect_server(!argv[1] ? "localhost":argv[1], !argv[2] ? "8080":argv[2]); 
	


	return 0;
}


