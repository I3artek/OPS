#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))
#define MAX_LINE 20

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s ([-t x] -n Name) ... \n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int x = 1;
    char c;
    while((c = getopt(argc, argv, "n:t:")) != -1)
    {
        if(c == '?')
        {
            usage(argv[0]);
        }
        else if(c == 'n')
        {
            for(int i = 0; i < x; i++)
            {
                printf("Hello %s\n", optarg);
            }
        }
        else if(c == 't')
        {
            int n = atoi(optarg);
            x = n;
        }
    }
    if(argc > optind)
    {
        usage(argv[0]);
    }
    return EXIT_SUCCESS;
}
