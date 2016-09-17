#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <sys/wait.h>



   



int connect_server(char * address, char * port)
{
	printf("Attempting to connect to: %s:%s\n\n", address, port);

	struct sockaddr_in saddr; //‐ the number to call
	struct hostent *hp; //‐ used to get number
	int n;
	int  addrlen;


	time_t start_t, end_t;
 	double diff_t;

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

	bcopy(hp->h_addr, (struct sockaddr *) &saddr.sin_addr, hp->h_length); printf("\t\tAssigned Address: %s\n", inet_ntoa(saddr.sin_addr));
	saddr.sin_port = htons(atoi(port)); printf("\t\tAssigned Port: %s\n", port);
	saddr.sin_family = AF_INET;

	printf("\tReady...\n");
	// Set up file descriptor set
	fd_set master;
	int maxfd;

	FD_ZERO(&master);
	FD_SET(STDIN_FILENO, &master);
	maxfd = STDIN_FILENO;

	fflush(stdout);

addrlen = sizeof(saddr);
	while(1)
	{
		fd_set readfds = master;
		char buf[100];
		int n, i;

		int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
		if(activity < 0)
			continue;

		for (int i = 0; i <= maxfd; i++)
		{
			if (FD_ISSET(i, &readfds))
			{
				if (i == STDIN_FILENO)
				{
					// Read in user input and write to the server
					buf[0]= 0x00;
					//bzero((void *)buf, sizeof(buf));
					//scanf("%100[^\n]%*c", buf);
					fgets(buf, 1000, stdin);
					buf[strlen(buf)-1]= 0x00;

					if(!strcmp(buf, "quit"))
					{
						getpeername(i , (struct sockaddr*)&saddr , (socklen_t*)&addrlen);
                        printf("Disconnected From Assigned:, ip %s , port %d \n" , inet_ntoa(saddr.sin_addr) , ntohs(saddr.sin_port));
						exit(0);
					}

					// Make connection to the server
					int sock = socket(AF_INET, SOCK_STREAM, 0);
					if(sock == -1) return -1;
					if (connect(sock, (struct sockaddr *) &saddr, sizeof(saddr)) != 0)
					{
						printf("Failed to connect.\n");
						break;
					}
					time(&start_t);
					send(sock, buf, strlen(buf), 0);

					FD_SET(sock, &master);
					if (sock > maxfd) maxfd = sock;

					printf("> ");
					fflush(stdout);
				}
				else
				{
					// Clear the line we will print on
					printf("\33[2K\r");

					// Read response from the server
					buf[0] = 0;
					//bzero((void *)&buf, sizeof(buf));
					n = recv(i, buf, sizeof(buf), 0);
					buf[n] = 0x00;
					if (n <= 0)
					{
						printf("Nothing...\n");
						time(&end_t);
						diff_t = difftime(end_t, start_t);
   						printf("Time  Taken = %f\n", diff_t);
					}
					else
					{
						char temp[1024];
						strcpy(temp, buf);

						char * cmd = NULL;
						cmd = strtok(temp, " ");
						
						printf("Response: ");
						fprintf(stdout, buf, strlen(buf));
						time(&end_t);
						diff_t = difftime(end_t, start_t);
   						printf("Time  Taken = %f\n", diff_t);
					}

					close(i);

					FD_CLR(i, &master);

					fflush(stdout);
				}
			}
		}
}
return 0;
}

