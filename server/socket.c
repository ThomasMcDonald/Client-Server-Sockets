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





extern int errno;
const int port = 8080;

int process_request(char *buffer, int sd);

int make_server_socket()
{

    struct sockaddr_in saddr;
    int  addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, valread , sd;
    int clientCounter;
    int max_sd;
    int n;
    int flags;
    char buffer[1025];
    fd_set readfds;


    printf("Initialising sd...\n");
    printf("\tCreating the sd...\n");





    int sock_id = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_id == -1) return -1;

    printf("\t\tSocket created.\n");
    bzero((void *)&saddr, sizeof(saddr));

    printf("\tSetting address...\n");

    saddr.sin_addr.s_addr = htonl(INADDR_ANY); printf("\t\tIP: ANY\n");
    saddr.sin_port = htons(port); printf("\t\t%s%d\n","PORT:",port);
    saddr.sin_family = AF_INET;

    printf("\tBinding to address...\n");

    if (bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        perror("bind failed. Error");
        return -1;
    }


    printf("\t\tSuccess.\n");
    printf("\tStarting Listener...\n");

    if (listen(sock_id, 5) != 0)
    {
        printf("\t\tFailed to start listener.\n");
        return -1;
    }

    for (int i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }

    addrlen = sizeof(saddr);
    puts("Waiting for connections ...");
    while(1) 
    {
                    //clear the sd set
        FD_ZERO(&readfds);

                    //add sock sd to set
        FD_SET(sock_id, &readfds);
        max_sd = sock_id;

                    //add child sockets to set
        for (int i = 0 ; i < max_clients ; i++) 
        {
                        //sd descriptor
            sd = client_socket[i];

                        //if valid sd descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

                        //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

                    //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }

                    //If something happened on the  sock , then its an incoming connection
        if (FD_ISSET(sock_id, &readfds)) 
        {
            if ((new_socket = accept(sock_id, (struct sockaddr *)&saddr, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            clientCounter++;
            printf("New Connection , Socket File Descriptor is %d , IP is : %s , Port : %d \n" , new_socket , inet_ntoa(saddr.sin_addr) , ntohs(saddr.sin_port));
            flags = fcntl(new_socket,F_GETFL,0);
            fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

            if ((send(new_socket, "Connected With Server\n", 10, 0)) < 0)
                printf("\tError sending initial command request.\n");

                       // puts("Welcome message sent successfully");

                        //add new sd to array of sockets
            for (int i = 0; i < max_clients; i++) 
            {
                            //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                                //printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
            }
        }


                    //else its some IO operation on some other sd :)
        for (int i = 0; i < max_clients; i++) 
        {
            bzero((void *)&buffer, sizeof(buffer));
            sd = client_socket[i];

            if (FD_ISSET(sd , &readfds)) 
            {
                  //Check if it was for closing , and also read the incoming message
                    valread = read( sd , buffer, 1024);
                    if (valread == 0)
                    {
                                //Somebody disconnected , get his details and print
                        getpeername(sd , (struct sockaddr*)&saddr , (socklen_t*)&addrlen);
                        printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(saddr.sin_addr) , ntohs(saddr.sin_port));
                        clientCounter--;
                                //Close the sd and mark as 0 in list for reuse
                        close(sd);
                        client_socket[i] = 0;
                        
                    }//
                    else
                    {
                        pid_t pid;

                        if ((pid = fork()) <0)
                        {
                            perror("fuck");
                        }
                        else if (pid == 0)
                        {
                            printf("Child %d has performed the following tasks:\n ",getpid());
                        if (!strcmp(buffer, "time"))
                        {
                            time_t t;
                            time(&t);

                            bzero((void *)&buffer, sizeof(buffer));
                            strcpy(buffer, "The time is: ");
                            strcat(buffer, ctime(&t));

                            int n = send(sd, buffer, strlen(buffer), 0);
                            if (n < 0)
                                printf("\tError sending time to client\n");
                            else
                                printf("\t%s%d\n","Sent time to client ",client_socket[i]);
                            exit(0);
                         
                        }
                        else if (!strcmp(buffer, "quit"))
                        {

                            getpeername(sd , (struct sockaddr*)&saddr , (socklen_t*)&addrlen);
                            //printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(saddr.sin_addr) , ntohs(saddr.sin_port));
                            clientCounter--;

                            bzero((void *)&buffer, sizeof(buffer));
                            strcpy(buffer, "Disconnected");
                            int n = send(sd, buffer, strlen(buffer), 0);        //Close the sd and mark as 0 in list for reuse
                            close(sd);
                            client_socket[i] = 0;
                            exit(0);
                           
                        }
                        else if (!strcmp(buffer, "count"))
                        {
                            bzero((void *)&buffer, sizeof(buffer));
                            strcpy(buffer, "Connected Clients: ");
                            strcat(buffer, "12");
                            int n = send(sd, buffer, strlen(buffer), 0);  
                            if (n < 0)
                                printf("\tError sending count to client\n");
                            else
                                printf("\t%s%d\n","Sent Client Count to client ",client_socket[i]);
                            exit(0);
                        }   
                        else
                        {

                            n = send(sd, "Error: Unknown Command\n", 24, 0);
                            if (n < 0)
                            {
                                printf("Error sending unknown command notification.\n");
                            }
                            printf("\tReceived an unknown command.\n");
                            exit(0);
                        }
                        }
                        else
                        {
                            int childexitstat;
                            if ((waitpid(pid, &childexitstat, 0)) < 0) perror("wait error");
                            printf("\t%s%s%d\n","Commited Sudoku ",WIFEXITED(childexitstat) ? "Normally " : "Abnornally ", WEXITSTATUS(childexitstat));
                        }
                    }
            }
        }
    }
    return 0;
}




