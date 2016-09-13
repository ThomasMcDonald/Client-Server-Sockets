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

	int connection = connect_server(argv[1], !argv[2] ? "8889":argv[2]); 
	if (connection < 0) exit(-1);
	printf("Ready!\n\n");

	while(1)
	{
		char buf[100];
		int n;

		// Read in the first message from the server ("Command: ")
		bzero((void *)&buf, sizeof(buf));
		n = recv(connection, buf, sizeof(buf), MSG_PEEK);

		if (n <= 0 || !(strcmp(buf,"Disconnected")))
		{
			printf("\nDisconnected.\n");
			close(connection);
			break;
		}
		fprintf(stdout, buf, n);

		

		// Read in user input and write to the server
		bzero((void *)&buf, sizeof(buf));
		scanf("%s", buf);
		if((send(connection, buf, strlen(buf), 0)) < 0)
			printf("%s\n","Message failed to send." );

		// Read response from the server
		bzero((void *)&buf, sizeof(buf));
		n = recv(connection, buf, sizeof(buf), 0);



	}

	return 0;
}


