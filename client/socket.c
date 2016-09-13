#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>





int connect_server(char * address, char * port)
{
	printf("Attempting to connect to: %s:%s\n\n", address, port);

	struct sockaddr_in saddr; //‐ the number to call
	struct hostent *hp; //‐ used to get number



	//‐‐ Step 1: Get a socket –
	printf("Initialising socket...\n");
	printf("\tCreating the socket...\n");
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) return -1;
	printf("\t\tSocket created.\n");



	//‐‐ Step 2: connect to server –
	bzero(&saddr, sizeof(saddr));
	printf("\tSetting address...\n");
	hp = gethostbyname(address); //‐ lookup host ip

	if (hp == NULL)
	{
		printf("\t\tCould not retrieve host info.\n");
		return -1;
	}

	bcopy(hp->h_addr, (struct sockaddr *) &saddr.sin_addr, hp->h_length); printf("\t\tGot address: %s\n", inet_ntoa(saddr.sin_addr));
	saddr.sin_port = htons(atoi(port)); printf("\t\tGot port: %s\n", port);
	saddr.sin_family = AF_INET;

	printf("\tAttempting connection...\n");

	if (connect(sock, (struct sockaddr *) &saddr, sizeof(saddr)) != 0)
	{
		printf("\t\tFailed to connect.\n");
		return -1;
	}
	printf("\t\tSuccess.\n");
	printf("Connection established.\n\n");

	return sock;

}

