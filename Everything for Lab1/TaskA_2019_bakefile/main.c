#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_LINE 32

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s (-p folderName) ... -r -s -n string -o outputFile \n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    FILE *recipe = fopen("Bakefile", "r");
    if(NULL == recipe)
    {
        ERR("fopen");
    }
    //FILE *files[10];
    //char filenames[10][MAX_LINE];
    char filename[MAX_LINE];
    char instruction[2];

    while(EOF != fscanf(recipe, "%s", instruction))
    {
        if(!strcmp(instruction, "+"))
        {
            if(EOF == fscanf(recipe, "%s", filename))
            {
                ERR("fscanf");
            }
        }
        else if(!strcmp(instruction, "-"))
        {
            if(EOF == fscanf(recipe, "%s", filename))
            {
                ERR("fscanf");
            }
        }
        else
        {
            
        }
        fprintf(stdout, "%s\n", instruction);
    }

    return EXIT_SUCCESS;
}
