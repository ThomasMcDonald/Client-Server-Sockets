#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static char *myStrDup (char *str) {
    char *other = malloc (strlen (str) + 1);
    if (other != NULL)
        strcpy (other, str);
    return other;
}

int main(int argc, char  *argv[])
{


    argv[argc+1] = NULL;

    execvp ();
	return 0;
}