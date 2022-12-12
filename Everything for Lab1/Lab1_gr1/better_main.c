#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s (-p folderName) ... -r -s -n string -o outputFile \n", pname);
    exit(EXIT_FAILURE);
}

extern char* optarg;
extern int optind;

int starts_with_string(char *a, char *b)
{
    return !strncmp(a, b, strlen(b));
}

void print_files_and_sizes(char* dirname);

//variables to keep track of activated flags
int r = 0;
int s = 0;
int n = 0;
int o = 0;
char *n_val = NULL;
char *out = NULL;
FILE *output = NULL;

int main(int argc, char **argv)
{
    //parse options to set proper flags
    char c;
    while((c = getopt(argc, argv, "p:rsn:o:")) != -1)
    {
        if('?' == c)
        {
            usage(argv[0]);
        }
        else if('o' == c)
        {
            o = 1;
            out = optarg;
        }
        else if('r' == c)
        {
            r = 1;
        }
        else if('s' == c)
        {
            s = 1;
        }
        else if('n' == c)
        {
            n = 1;
            n_val = optarg;
        }
    }
    //check for proper number of arguments
    if(argc > optind)
    {
        usage(argv[0]);
    }
    //set the output file descriptor to the file or stdout
    if(o)
    {
        output = fopen(out, "w");
        if(NULL == output)
        {
            ERR("fopen");
        }
    }
    else
    {
        output = stdout;
    }
    //now process the directories with the proper flags applied
    //we can safely omit error checking now
    //set the options index to 1 to start from the first option again
    optind = 1;
    while((c = getopt(argc, argv, "p:rsn:o:")) != -1)
    {
        if(c == 'p')
        {
            print_files_and_sizes(optarg);
        }
    }
    //close the file if it was opened
    if(stdout != output)
    {
        fclose(output);
    }

    return EXIT_SUCCESS;
}

void print_files_and_sizes(char* dirname)
{
    //we skip the whole directory if it starts with provided string
    if(n && starts_with_string(dirname, n_val))
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
    if(NULL == (dirp = opendir(".")))
    {
        //check for access and continue in case of an error
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        //check if it is a proper folder and continue in case of an error
        else if(ENOENT == errno || ENOTDIR == errno)
        {
            fprintf(stderr, "\"%s\" is not a valid folder\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        //for other error crash the program <3
        else
        {
            ERR("opendir");
        }
    }
    //print the name of the folder
    //or print the absolute path
    fprintf(output, "path: %s\n", dirname);
    //fprintf(output, "path: %s\n", getcwd(NULL, 0));

    //loop through the contents of the folder
    while(NULL != (dp = readdir(dirp)))
    {
        //s == 1 => stat performed - following the link
        //s == 0 => lstat performed - not following the link
        if(s && stat(dp->d_name, &filestat))
        {
            ERR("stat");
        }
        else if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        //ommit the directories . and ..
        if(strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
        {
            //check if the file name starts with the given string if n == 1
            if(!n || !starts_with_string(dp->d_name, n_val))
            {
                fprintf(output, "%s %lld\n", dp->d_name, filestat.st_size);
            }
        }

    }
    //perform the recursion on all the subfolders if r == 1
    if(r)
    {
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
                print_files_and_sizes(dp->d_name);
            }
        }
    }
    if(closedir(dirp))
    {
        ERR("closedir");
    }
    chdir(starting);
    free(starting);
}
