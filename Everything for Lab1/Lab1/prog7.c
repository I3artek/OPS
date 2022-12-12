#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

int main(int argc, char **argv)
{
    int i = 0;
    while(environ[i] != NULL)
    {
        printf("%s\n", environ[i++]);
    }
    return EXIT_SUCCESS;
}
