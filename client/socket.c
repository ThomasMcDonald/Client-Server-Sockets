#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

//



int connect_server(char * address, char * port)
{
	printf("Attempting to connect to: %s:%s\n\n", address, port);

	struct sockaddr_in saddr; //‐ the number to call
	struct hostent *hp; //‐ used to get number
	int n;
	struct timeval t1, t2;
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

	int opt = 1;  
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;  
    int max_sd;   
        
    char buffer[1000];  //data buffer of 1K 
        
    //set of socket descriptors 
    fd_set master, readfds;    

    //clear the socket set 
    FD_ZERO(&master);  
    
        //add master socket to set 
    FD_SET(STDIN_FILENO, &master);  
    max_sd = STDIN_FILENO;
    client_socket[0] = max_sd;  

    printf("Added STDIN_FILENO to FD SET as %d\n", max_sd);
        
	fflush(stdout);
	int currentfds = 1;

	for (int j = 1; j < max_clients; j++)
		client_socket[j] = 0;

	while(1)  
	{  
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
		readfds = master;
		//printf("Max_sd before select = %d\n", max_sd);
		
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);  

		if (activity < 0)  
		{  
			printf("select error");
			exit(4);  
		}  

		for (i = 0; i < sizeof(master); i++)  
		{  
			sd = client_socket[i]; 

			if (FD_ISSET(sd, &readfds))
			{
				if (sd == STDIN_FILENO)
				{

					// Read in user input and write to the server
					buffer[0]=0x00;
					scanf("%256[^\n]%*c", buffer);

					if(!strcmp(buffer, "quit"))
					{
						exit(0);
					}

					// Make connection to the server
					int sock = socket(AF_INET, SOCK_STREAM, 0);
					if(sock == -1)
					{
						perror("Error creating socket...\n");
						exit(1);
					}

					if (connect(sock, (struct sockaddr *) &saddr, sizeof(saddr)) != 0)
					{
						printf("Failed to connect to server...\n");
						exit(1);
					}

					n = send(sock, buffer, strlen(buffer), 0);
					gettimeofday(&t1, NULL);
					if (n < 0)
					{
						perror("Send Failed...");
						break;
					}
					


					FD_SET(sock, &master);
					currentfds++;
					for (int z = i; z < max_clients; z++)  
					{  
                		//if position is empty 
						if( client_socket[z] == 0 )  
						{  
							client_socket[z] = sock;  
							printf("Adding to list of sockets as %d\n" , client_socket[z]);
							if (sock > max_sd)
								max_sd = sock;  
							break;  
						}  
					}
					fflush(stdout);
					break;
				}
				else
				{

				    char * temp;
				    char cmd[200];
				    char args[5][200];
				    int argc;
				    int n, i;

				    bzero((void *)cmd, sizeof(cmd));
				    bzero((void *)args, 5*sizeof(char));

				    if ((temp = strtok(buffer, " ")) != NULL)
				        strcpy(cmd, temp);

				    temp = strtok(NULL, " "); i = 0;
				    while (temp != NULL && i < 5)
				    {
				        strcpy(args[i], temp); i++; argc++;
				        temp = strtok(NULL, " ");
					}
					printf("\33[2K\r");


					bzero((void *)buffer, sizeof(buffer));
					n = recv(sd, buffer, sizeof(buffer), 0);
					buffer[n] = 0;
					if (n <= 0)
					{
						printf("Nothing received from server...\n");
					}
					else if (!strcmp(buffer, "check")){

						char  *filename = args[0];
						char line[256];
					 	int one;
					 	int two;
					 	send(sd,"Accepted",10,0);
					 	printf("\tUploading file: %s\n", filename);
							FILE * input = fopen(filename,"r");if (input == NULL)perror("File didnt open\n");
							while(fgets(line, sizeof(line), input) != NULL)
								{
								one = send(sd,line, strlen(line),0);if(one <0)perror("Didnt work send\n");
								two = recv(sd, buffer, strlen(buffer), 0);if(two<0)perror("Recv no work\n");
								bzero((void *)buffer, sizeof(buffer));
								}
								bzero((void *)buffer, sizeof(buffer));
								printf("\tFinished Uploading.\n");
								fclose(input);
					}  
					else if (!strcmp(buffer, "getCheck")){
						int exists; 
						int force;
						char *filepath = NULL;
						char *newfile = NULL;
						if(!strcmp(args[1],"-f"))
				        {
				            filepath = args[0];
				            if (argc > 3)
				                newfile = args[2];
				            force = 1;
				        }
				        else{
				            filepath = args[0];
				            if (argc < 2)
				                newfile = NULL;
				            force = 0;
				        }

						char fileline[256];
						int n;
						FILE * output;
						printf("getChecked\n");
						n = send(sd,"Accepted",8,0); if(n<0)printf("didnt work");
					                printf("Accepted\n");
					                if(newfile != NULL){
					                output = fopen(newfile,"w"); if (output == NULL)perror("File didnt open\n");
					            	}
					                int a;   
					               	 n = recv(sd,fileline,sizeof(fileline),0);
					               	 n = send(sd, "received", 8, 0);
					               	 //printf("Firstline = %s\n", fileline);
					               	 int count = 0;
					                while(n != 0)
					                {

					                	if(newfile == NULL)printf("%s",fileline);
					                	else fwrite(fileline, sizeof(char), strlen(fileline), output);
					                	 
						              	bzero((void *)fileline, sizeof(fileline));
						            	n = recv(sd, fileline, sizeof(fileline), 0); if(n<0){perror("Didnt Recieve\n");}
						            	count++;
						            	if(count == 40 && newfile == NULL)
						            	{
						            		printf("Press any key to continue\n");
						            		count =0;
						            		getchar();
						            	}
						                a = send(sd,"Recieved Line\n",14,0); if(a<0){printf("shit\n"); break;}
						                //printf("%s\n",fileline);
						              	
					                }
					                
					                if(newfile == NULL) printf("Fin.\n");
					                else {fclose(output); printf("Fin.\n");}

					}  
					
					else
					{
						
						printf("Response: \n\t");
						fprintf(stdout, buffer, strlen(buffer));
				


					}

					gettimeofday(&t2, NULL);
					long elapsed = ((t2.tv_sec - t1.tv_sec)*1000000 + t2.tv_usec - t1.tv_usec) / 1000;
					printf("Time Taken: %li\n",elapsed);

					close(sd);
					FD_CLR(sd, &master);

					client_socket[i] = 0; 
					currentfds--; 

					fflush(stdout);
					break;
				}
			}
		}
	}   

	return 0;
}
//

//







