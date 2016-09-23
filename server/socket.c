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
#include <sys/stat.h>




extern int errno;
const int port = 8080;
int list(int);
int longlist(int);
int sortls(int);
int sortlslong(int);
int greplist(int,char*,char*);

int process_request(int socket){
    char buf[1024];
    char buffed[1024];
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

    strcpy(buffed,buf);
    if (n <= 0)
    {
        printf("\tError receiving input from client or connection was forcefully closed!\n");
    }
    //puts(buf);
    // Split the input from the client into command and arguments
    if ((temp = strtok(buf, " ")) != NULL)
        strcpy(cmd, temp);

    temp = strtok(NULL, " "); i = 0;
    while (temp != NULL && i < 5)
    {
        strcpy(args[i], temp); i++; argc++;
        temp = strtok(NULL, " ");
    }

    if(strpbrk(buffed,"|") > 0)
    {
        if(!strcmp(cmd,"ls"))
        {
            if (!strcmp(args[0], "-l"))
            {
                if(!strcmp(args[2], "sort"))
                    printf("\tSent sorted ls -l to client\n");
                    sortlslong(socket);
            }
            else {
                if(!strcmp(args[1], "sort"))
                    printf("\tSent sorted ls to client\n");
                    sortls(socket);
            }
        }
        else if(!strcmp(cmd,"get"))
        {
           
            char *filename = args[0];
            char *word = args[3];
            greplist(socket,filename,word);
        }
    }
    else if(!strcmp(cmd, "delay"))
    {
        int stringvalue = atoi(args[0]);
        sleep(stringvalue);
        strcat(args[0],"\n");
        n = send(socket,args[0],5,0);
        if (n < 0)
            printf("\tError sending  message to client.\n");
        else
            printf("\tDelayed Client for %d Seconds\n",stringvalue);

        return 0;
    }
    else if(!strcmp(cmd,"sys"))
    {
        bzero((void *)buf, sizeof(buf));

        n = uname(&OS);
        if(n<0)
        {
            printf("Error Retrieving System Information\n");
            n = send(socket,"Error Retrieving System Information\n",36,0);
            if (n < 0)
            printf("\tError sending  message to client.\n");
             else
            printf("\tSent Error Message to client\n");

        }
        else
        {
        strcpy(buf,"\n\tOS: ");
        strcat(buf,OS.sysname);
        strcat(buf,"\n\tVersion: ");
        strcat(buf,OS.version);
        strcat(buf,"\n\tCPU: ");
        strcat(buf,OS.machine);
        strcat(buf,"\n");


        n = send(socket,buf,strlen(buf),0);
        if (n < 0)
            printf("\tError sending  message to client.\n");
             else
            printf("\tSent OS information to client\n");
        }
    }
    else if(!strcmp(cmd, "ls"))
    {
        if (!strcmp(args[0], "-l"))
        {
            printf("\tSent ls -l to client\n");
            longlist(socket);
        }
        else
        {
            printf("\tSent ls to client\n");
            list(socket);
        }
        return 0;
    }
    else if(!strcmp(cmd, "put"))
    { 
        int force;
        int exists;
        FILE * output;
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
            newfile = args[1];
            force = 0;
        }
        printf("\tReceiving file: %s\n", filepath);
        printf("\t\tRename to: %s\n", newfile);
        printf("\t\tForced: %s\n", (force ? "true":"false"));

        // Check if the file already exists
        struct stat st;
        exists = stat(filepath, &st);
        printf((exists == 0 ? "\t\tFile Exists\n":"\t\tFile Does Not Exist\n"));

        if (!exists && !force)
        {
            n = send(socket, "The file already exists and you have not specified force [-f].\n", 65, 0);
            if (n < 0)
                printf("\tError sending error message to client.\n");
            else
                printf("\tSent file exists error message to client.\n");

            return 0;
        }
        else{

            char filelist[1024];
            send(socket,"check",5,0);
            if((recv(socket,buf, sizeof(buf),0))<0)perror("doesnt work\n");
            else{
                if(!strcmp(buf,"Accepted"))
                {
                    printf("Uploading File...\n");
                    if(newfile != NULL){output = fopen(newfile,"w"); if(output == NULL)perror("File didnt open\n");}
                    else{output = fopen(filepath,"w"); if(output == NULL)perror("File didnt open2\n");}
                    int a;
                    n = recv(socket,filelist,sizeof(filelist),0);
                    while(n != 0)
                    {
                        fputs(filelist,output);
                         bzero((void *)filelist, sizeof(filelist));
                        a = send(socket,"Recieved Line\n",20,0); if(a<0){printf("shit\n"); break;}
                        n = recv(socket, filelist, sizeof(filelist), 0); if(n<0 && n != 0){perror("Didnt Recieve\n");}
                    }
                    printf("Fin.\n");
                    fclose(output);
                }
            }
        }
            cmd[0]=0x00;
            return 0;
    }
    else if(!strcmp(cmd, "get"))
    { 
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
            newfile = args[1];
            force = 0;
        }
        printf("\tReceiving file: %s\n", filepath);
        printf("\t\tRename to: %s\n", newfile);
        printf("\t\tForced: %s\n", (force ? "true":"false"));

        // Check if the file already exists
        struct stat st;
        char *fullpath;

        exists = stat(filepath, &st);
        printf((exists == 0 ? "\t\tFile Exists\n":"\t\tFile Does Not Exist\n"));

        if (exists && !force)
        {
            n = send(socket, "The file already exists and you have not specified force [-f].\n", 65, 0);
            if (n < 0)
                printf("\tError sending error message to client.\n");
            else
                printf("\tSent file exists error message to client.\n");

            return 0;
        }
        else{
        printf("Checked\n");
        bzero((void *)buf, sizeof(buf));
        char line[256];
        int one;
        int two;
        send(socket,"getCheck",10,0);

        recv(socket,buf, sizeof(buf),0);
        printf("%s\n",buf);
        if(!strcmp(buf,"Accepted")){
            bzero((void *)buf, sizeof(buf));
            FILE * input = fopen(filepath,"r");if (input == NULL)printf("File didnt open\n");
            while(fgets(line, sizeof(line), input) != NULL)
                {
                //printf("Line sent = %s\n", line);
                 one = send(socket,line, strlen(line),0);if(one <0)perror("Didnt work send\n");
                 two = recv(socket, buf, sizeof(buf), 0);if(two<0)perror("Recv no work\n");
                 //printf("%s",line);
                //bzero((void *)line, sizeof(line));
                 bzero((void *)buf, sizeof(buf));
                }
                printf("fin.\n");
                fclose(input);
    }}

}
    else
    {
        n = send(socket,"Unknown command\n",16,0);
        if (n < 0)
            printf("\tError sending message to client.\n");
        else
            printf("\tUnknown command recieved\n");

        return 0;
    }
    return 0;
}

int make_server_socket(char * address, char * port)
{

    struct sockaddr_in saddr;
    struct hostent *hp;


    hp = gethostbyname(address);


    printf("Initialising sd...\n");
    printf("\tCreating the sd...\n");


    int sock_id = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_id == -1) return -1;

    printf("\t\tSocket created.\n");
    bzero((void *)&saddr, sizeof(saddr));

    printf("\tSetting address...\n");

    bcopy(hp->h_addr, (struct sockaddr *) &saddr.sin_addr, hp->h_length); printf("\t\tAssigned Address: %s\n", inet_ntoa(saddr.sin_addr));
    saddr.sin_port = htons(atoi(port)); printf("\t\t%s%s\n","PORT:",port);
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


int list(int socket)
{
    char *test[] = {"/bin/ls", NULL};

    //int fd;
    //fd = open("temp.txt", O_WRONLY | O_CREAT | O_TRUNC);

    dup2(socket, STDOUT_FILENO);
    dup2(socket, STDERR_FILENO);
                   
    //close(fd);     

    execv("/bin/ls", test);

    return 1;
}

int longlist(socket)
{
    char *test[] = {"/bin/ls","-l", NULL};

    //int fd;
    //fd = open("temp.txt", O_WRONLY | O_CREAT | O_TRUNC);

    dup2(socket, STDOUT_FILENO);
    dup2(socket, STDERR_FILENO);
                 

    execv("/bin/ls", test);

    return 1;
}

int sortls(int socket){
    char *test[] = {"ls",NULL};
    char *test2[] = {"sort",0};
    int p[2];
    pipe(p);
    if (fork() == 0) {
        // first child, run ls, so its output(fd 1) should be redirected to a pipe end
        dup2(p[1], STDOUT_FILENO);
        close(p[0]); // just for safety
        close(p[1]); // just for safety
        execvp(test[0], test);
    }

    if (fork() == 0) {
        // second child, run sort, so its input(fd 0) should be redirected to another pipe end
        dup2(p[0], STDIN_FILENO);
        close(p[0]); // just for safety
        close(p[1]); // just for safety

        dup2(socket, STDOUT_FILENO);
        dup2(socket, STDERR_FILENO);

        execvp(test2[0],test2);
    }

    close(p[0]);
    close(p[1]);
    return 1;
}

int sortlslong(int socket){
    char *test[] = {"ls","-l",NULL};
    char *test2[] = {"sort",0};
    int p[2];
    pipe(p);
    if (fork() == 0) {
        // first child, run ls, so its output(fd 1) should be redirected to a pipe end
        dup2(p[1], STDOUT_FILENO);
        close(p[0]); // just for safety
        close(p[1]); // just for safety
        execvp(test[0], test);
    }

    if (fork() == 0) {
        // second child, run sort, so its input(fd 0) should be redirected to another pipe end
        dup2(p[0], STDIN_FILENO);
        close(p[0]); // just for safety
        close(p[1]); // just for safety

        dup2(socket, STDOUT_FILENO);
        dup2(socket, STDERR_FILENO);

        execvp(test2[0],test2);
    }
    
    close(p[0]);
    close(p[1]);
    return 1;
}

int greplist(int socket,char *abspath,char *lookup){
    char *test[] = {"grep",lookup,abspath,NULL};
    int p[2];
    pipe(p);

 if (fork() == 0) {
        // second child, run sort, so its input(fd 0) should be redirected to another pipe end
        dup2(p[0], STDIN_FILENO);
        close(p[0]); // just for safety
        close(p[1]); // just for safety

        dup2(socket, STDOUT_FILENO);
        dup2(socket, STDERR_FILENO);

        execvp(test[0],test);
    }

return 1;

}

