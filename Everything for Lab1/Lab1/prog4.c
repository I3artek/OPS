#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE 20

int main(int argc, char **argv)
{
    for(int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }
    return EXIT_SUCCESS;
}
