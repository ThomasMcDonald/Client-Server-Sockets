#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>




int make_server_socket(char * address, char * port);
int process_request(int socket);
void handle_sigchld(int sig) {
  int saved_errno = errno;
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
  errno = saved_errno;
}



int main(int argc, char *argv[])
{
	pid_t pid;

	// Handle SIGCHLD by calling reap
	struct sigaction sigchld_action;
	memset(&sigchld_action, 0, sizeof (sigchld_action));
	sigchld_action.sa_handler = &handle_sigchld;
	sigaction(SIGCHLD, &sigchld_action, 0);

	printf("Starting the server...\n\n");


	int sock = make_server_socket(!argv[1] ? "localhost":argv[1], !argv[2] ? "8080":argv[2]);

	while(1)
	{
		int csock = accept(sock, NULL, NULL);
		if (csock < 0)
		{
			continue;
		}
		
		if((pid = fork()) == 0)
		{
			close(sock);

			process_request(csock);

			exit(0);
		}

		close(csock);
	}
	
	close(sock);
	printf("Server stopped.\n");

	return 0;
}



