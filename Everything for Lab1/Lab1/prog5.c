#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))
#define MAX_LINE 20

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s name times>0\n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        usage(argv[0]);
    }
    char **endPtr = NULL;
    int t = strtol(argv[2], endPtr, 10);
    if(0 == t && NULL == endPtr)
    {
        usage(argv[0]);
    }
    for(int i = 0; i < t; i++)
    {
        printf("Hello %s\n", argv[1]);
    }
    return EXIT_SUCCESS;
}
