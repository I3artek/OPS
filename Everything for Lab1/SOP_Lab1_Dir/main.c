#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <ftw.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s (-p folderName) ... -r -s -n string -o outputFile \n", pname);
    exit(EXIT_FAILURE);
}

void print_files_and_sizes(char* dirname, int depth);

FILE *output;
int out = 0;

int main(int argc, char **argv)
{
    output = stdout;
    char *dirname = getenv("DIR");
    int depth = atoi(getenv("DEPTH"));
    out = !strcmp(getenv("OUTPUT"), "yes");

    print_files_and_sizes(dirname, depth);

    return EXIT_SUCCESS;
}

void print_files_and_sizes(char* dirname, int depth)
{
    if(0 == depth)
    {
        return;
    }
    //save the current directory path
    char *starting = NULL;
    if((starting = getcwd(starting, 0)) == NULL)
    {
        ERR("getcwd");
    }
    //declare the necessary variables
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    struct stat filestat;
    //change current directory and open it
    chdir(dirname);
    if(out)
    {
        output = fopen(".sizes", "w");
        if(NULL == output)
        {
            ERR("fopen");
        }
    }
    if(NULL == (dirp = opendir(".")))
    {
        //check for access and continue in case of an error
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            if(stdout != output)
            {
                fclose(output);
            }
            chdir(starting);
            free(starting);
            return;
        }
        else
        {
            ERR("opendir");
        }
    }
    //print the name of the folder
    //or print the absolute path
    fprintf(output, "Directory: %s\n", dirname);
    fprintf(output, "Directory analysis:\n");

    //loop through the contents of the folder to find the max
    long long max_size = 0;
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        if(S_ISREG(filestat.st_mode))
        {
            if(filestat.st_size > max_size)
            {
                max_size = filestat.st_size;
            }
        }
    }
    //loop to print
    rewinddir(dirp);
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        if(S_ISREG(filestat.st_mode))
        {
            fprintf(output, "%s: %lld", dp->d_name, filestat.st_size);
            if(filestat.st_size == max_size)
            {
                fprintf(output, " - BIGGEST");
            }
            fprintf(output, "\n");
        }
    }
    if(stdout != output)
    {
        fclose(output);
    }
    //perform the recursion on all the subfolders with less depth
    //rewind the directory stream iterator to the beginning
    rewinddir(dirp);
    //reads the contents again
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        //check if it is a folder, while ommiting . and ..
        if(S_ISDIR(filestat.st_mode) && strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
        {
            print_files_and_sizes(dp->d_name, depth - 1);
        }
    }

    if(closedir(dirp))
    {
        ERR("closedir");
    }
    chdir(starting);
    free(starting);
}
