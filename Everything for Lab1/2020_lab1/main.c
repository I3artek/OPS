#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define X_LINE "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE of %s is different!\n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if(!strcmp(argv[1], "create"))
    {
        //check if user provided enough arguments
        if(NULL == argv[2])
        {
            usage(argv[0]);
        }
        //check for -l flag
        int l = strcmp(argv[2], "-l") ? 0 : 1;
        //l == 0 => argv[2] == filename
        //l == 1 => argv[3] == filename
        //argv[2 + l] == filename

        //check if user provided enough arguments
        if(NULL == argv[2 + l])
        {
            usage(argv[0]);
        }

        //check if the file exists
        errno = 0;
        FILE *db = fopen(argv[2 + l], "r");
        if(db)
        {
            //exit if the file exists
            fprintf(stderr, "The file %s already exists\n", argv[2 + l]);
            exit(EXIT_FAILURE);
        }
        else if(ENOENT != errno)
        {
            //error other than file not existing occured
            ERR("fopen");
        }
        //if the file does not exist:
        //open the file for writing
        db = fopen(argv[2 + l], "w");
        //write appropriate lines
        for(int i = 3 + l; i < argc; i++)
        {
            fprintf(db, "%-32s%-32d\n", argv[i], 0);
        }
        if(l)
        {
            fprintf(db, "%s\n", X_LINE);
        }
        fclose(db);
    }
    else if(!strcmp(argv[1], "show"))
    {
        //
        if(3 != argc)
        {
            usage(argv[0]);
        }
        //open the file and check for errors
        errno = 0;
        FILE *db = fopen(argv[2], "r");
        if(NULL == db)
        {
            ERR("fopen");
        }
        fprintf(stdout, "Database name: %s\n", argv[2]);
        //buffer arrays for options and votes
        char option[33];
        char votes[33];
        int locked = 0;
        while(EOF != fscanf(db, "%s%s", option, votes))
        {
            if(!strcmp(option, X_LINE))
            {
                locked = 1;
                break;
            }
            fprintf(stdout, "%-32s%s\n", option, votes);
        }
        fprintf(stdout, "Database is %s\n", locked ? "locked" : "unlcoked");
        fclose(db);
    }
    else if(!strcmp(argv[1], "vote"))
    {
        int r = 0;
        if(5 == argc && !strcmp(argv[2], "-r"))
        {
            r = 1;
        }
        else if(4 != argc)
        {
            usage(argv[0]);
        }
        //r == 0 => argv[2] == filename
        //r == 1 => argv[3] == filename
        //argv[2 + r] == filename

        //check if the file exists
        errno = 0;
        FILE *db = fopen(argv[2 + r], "r");
        if(NULL == db && ENOENT == errno)
        {
            //exit if the file does not exist
            fprintf(stderr, "The file %s does not exist\n", argv[2 + r]);
            exit(EXIT_FAILURE);
        }
        else if(NULL == db)
        {
            ERR("fopen");
        }
        fclose(db);
        db = fopen(argv[2 + r], "r+");
        char option[33];
        char votes[33];
        int locked = 0;
        while(EOF != fscanf(db, "%s%s", option, votes))
        {
            if(!strcmp(option, X_LINE))
            {
                locked = 1;
                break;
            }
            if(!strcmp(option, argv[3 + r]))
            {
                //decrease votes if -r was provided and votes > 1
                if(r && atoi(votes) > 0)
                {
                    fseek(db, -strlen(votes), SEEK_CUR);
                    fprintf(db, "%-32d", atoi(votes) - 1);
                }
                //increase if -r not provided
                else if(!r)
                {
                    fseek(db, -strlen(votes), SEEK_CUR);
                    fprintf(db, "%-32d", atoi(votes) + 1);
                }
                //set locked = 1 to avoid adding at the end
                locked = 1;
                break;
            }
        }
        if(!locked && !r)
        {
            fprintf(db, "%-32s%-32d\n", argv[3], 1);
        }

        fclose(db);
    }

    return EXIT_SUCCESS;
}
