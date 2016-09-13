#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>


int make_server_socket();
int process_request(int fd);


int main(int argc, char const *argv[])
{

	printf("Starting the server...\n\n");
	make_server_socket();

	return 0;
}



