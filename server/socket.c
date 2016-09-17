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

int process_request(int socket){
    char buf[1024];
    char * temp;
    char cmd[200];
    char args[5][200];
    int argc;
    int n, i;
    struct utsname OS;

    bzero((void *)buf, sizeof(buf));
    bzero((void *)cmd, sizeof(cmd));
    bzero((void *)&args, 5*sizeof(char));
    n = recv(socket, buf, sizeof(buf), 0);
    buf[n] = 0x00;
    if (n <= 0)
    {
        printf("\tError receiving input from client or connection was forcefully closed!\n");
    }
    puts(buf);
    // Split the input from the client into command and arguments
    if ((temp = strtok(buf, " ")) != NULL)
        strcpy(cmd, temp);

    temp = strtok(NULL, " "); i = 0;
    while (temp != NULL && i < 5)
    {
        strcpy(args[i], temp); i++; argc++;
        temp = strtok(NULL, " ");
    }
    if (!strcmp(cmd, "litfam"))
    {
        n = send(socket, "So lit Fam\n", 13, 0);
        if (n < 0)
            printf("\tError sending  message to client.\n");
        else
            printf("\tSent message to client.\n");

        return 0;
    }

    else if(!strcmp(cmd, "delay"))
    {
        int stringvalue = atoi(args[0]);
        sleep(stringvalue);
        strcat(args[0],"\n");
        n = send(socket,args[0],20,0);
        if (n < 0)
            printf("\tError sending  message to client.\n");
        else
            printf("\tDelayed Client for %d Seconds\n",stringvalue);

        return 0;
    }
    else if(!strcmp(cmd,"sys"))
    {
        printf("sys\n");
        buf[0] = 0;
        //bzero((void *)buf, sizeof(buf));
        n = uname(&OS);
        if(n<0)
        {
            printf("Error Retrieving System Information\n");
            n = send(socket,"Error Retrieving System Information\n",100,0);
            if (n < 0)
            printf("\tError sending  message to client.\n");
             else
            printf("\tSent Error Message to client\n");

        }
        else{
        printf("sys2\n");
        strcpy(buf,"\n\tOS: ");
        strcat(buf,OS.sysname);
        strcat(buf,"\n\tVersion: ");
        strcat(buf,OS.version);
        strcat(buf,"\n\tCPU: ");
        strcat(buf,OS.machine);
        strcat(buf,"\n");

        printf("Buf = %s %d\n", buf, strlen(buf));

        n = send(socket,buf,strlen(buf),0);
        if (n < 0)
            printf("\tError sending  message to client.\n");
             else
            printf("\tSent Error Message to client\n");
    }

    }
    else if(!strcmp(cmd, "ls"))
    {
        if (!strcmp(args[0], "-l")){
            strcat(args[0]," Long listed fam\n");
            
            n = send(socket,args[0],20,0);
            if (n < 0)
                printf("\tError sending  message to client.\n");
            else
                printf("\tLong Listed Fam\n");
        }
        return 0;
    }
    else{
        n = send(socket,"Unknown command\n",16,0);
        if (n < 0)
            printf("\tError sending message to client.\n");
        else
            printf("\tUnknown command recieved\n");

        return 0;
    }
    return 0;
}

int make_server_socket()
{

    struct sockaddr_in saddr;



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
        exit(0);
    }



    printf("\tStarting Listener...\n");

    if (listen(sock_id, 5) != 0)
    {
        printf("\t\tFailed to start listener.\n");
        return -1;
    }
    printf("\t\tSuccess.\n");

    puts("Waiting for connections ...");
    return sock_id;
}




