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

int starts_with_string(char *filename, char *n_val)
{
    return !strncmp(filename, n_val, strlen(n_val));
}

int file_accesed = 0;

void print_files(char* dirname);
void print_files_and_sizes_n(char* dirname, int n, char *n_val);
void print_files_and_sizes_n_r_s(char* dirname, int n, char *n_val, int r, int s);
void print_files_and_sizes_n_r_s_o(char* dirname, int n, char *n_val, int r, int s, char *out);

int main(int argc, char **argv)
{
    int r = 0, s = 0;
    int n = 0;
    int o = 0;
    char *out = NULL;
    char *n_val = NULL;
    char c;
    //parse options to set proper flags
    while((c = getopt(argc, argv, "p:rsn:o:")) != -1)
    {
        if(c == '?')
        {
            usage(argv[0]);
        }
        else if(c == 'o')
        {
            o = 1;
            //we concatenate the string in order to get full path
            out = strcat(strcat(getcwd(NULL, 0), "/"), optarg);
        }
        else if(c == 'r')
        {
            r = 1;
        }
        else if(c == 's')
        {
            s = 1;
        }
        else if(c == 'n')
        {
            n = 1;
            n_val = optarg;
        }
    }
    if(argc > optind)
    {
        usage(argv[0]);
    }
    //now process the directories with the proper flags applied
    //we can safely omit error checking now
    optind = 1;
    while((c = getopt(argc, argv, "p:rsn:o:")) != -1)
    {
        if(c == 'p')
        {
            //part 1.
            //print_files(optarg);
            //part 2.
            //print_files_and_sizes_n(optarg, n, n_val);
            //part 3.
            //print_files_and_sizes_n_r_s(optarg, n, n_val, r, s);
            //part 4.
            if(o)
            {
                print_files_and_sizes_n_r_s_o(optarg, n, n_val, r, s, out);
            }
            else
            {
                print_files_and_sizes_n_r_s(optarg, n, n_val, r, s);
            }
        }
    }

    return EXIT_SUCCESS;
}

//Simply print
void print_files(char* dirname)
{
    char *starting = NULL;
    if((starting = getcwd(starting, 0)) == NULL)
    {
        ERR("getcwd");
    }
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    struct stat filestat;
    chdir(dirname);
    if(NULL == (dirp = opendir(".")))
    {
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        else if(ENOENT == errno || ENOTDIR == errno)
        {
            fprintf(stderr, "\"%s\" is not a valid folder\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        else
        {
            ERR("opendir");
        }
    }
    fprintf(stdout, "path: %s\n", dirname);
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        //we omit . and ..
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
        {
            continue;
        }
        fprintf(stdout, "%s\n", dp->d_name);
    }
    if(closedir(dirp))
    {
        ERR("closedir");
    }
    chdir(starting);
    free(starting);
}

//include option -n and print sizes
void print_files_and_sizes_n(char* dirname, int n, char *n_val)
{
    //not sure about this if
    if(n && starts_with_string(dirname, n_val))
    {
        return;
    }
    char *starting = NULL;
    if((starting = getcwd(starting, 0)) == NULL)
    {
        ERR("getcwd");
    }
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    struct stat filestat;
    chdir(dirname);
    if(NULL == (dirp = opendir(".")))
    {
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        else if(ENOENT == errno || ENOTDIR == errno)
        {
            fprintf(stderr, "\"%s\" is not a valid folder\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        else
        {
            ERR("opendir");
        }
    }
    fprintf(stdout, "path: %s\n", dirname);
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        //we omit . and ..
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
        {
            continue;
        }
        //include the -n flag
        if(!n || !starts_with_string(dp->d_name, n_val))
        {
            fprintf(stdout, "%s %lld\n", dp->d_name, filestat.st_size);
        }
    }
    if(closedir(dirp))
    {
        ERR("closedir");
    }
    chdir(starting);
    free(starting);
}

void print_files_and_sizes_n_r_s(char* dirname, int n, char *n_val, int r, int s)
{
    //not sure about this if
    if(n && starts_with_string(dirname, n_val))
    {
        return;
    }
    char *starting = NULL;
    if((starting = getcwd(starting, 0)) == NULL)
    {
        ERR("getcwd");
    }
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    struct stat filestat;
    chdir(dirname);
    if(NULL == (dirp = opendir(".")))
    {
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        else if(ENOENT == errno || ENOTDIR == errno)
        {
            fprintf(stderr, "\"%s\" is not a valid folder\n", dirname);
            chdir(starting);
            free(starting);
            return;
        }
        else
        {
            ERR("opendir");
        }
    }
    fprintf(stdout, "path: %s\n", dirname);
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        //include the -s flag
        if(s && S_ISLNK(filestat.st_mode))
        {
            stat(dp->d_name, &filestat);
        }
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            //include the -n flag
            if(!n || !starts_with_string(dp->d_name, n_val))
            {
                fprintf(stdout, "%s %lld\n", dp->d_name, filestat.st_size);
            }
        }

    }
    //now we perform the recursion on all the subfolders
    if(r)
    {
        rewinddir(dirp);
        while(NULL != (dp = readdir(dirp)))
        {
            if(lstat(dp->d_name, &filestat))
            {
                ERR("lstat");
            }
            //we have ommit . and .. to avoid looping
            if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            {
                continue;
            }
            if(S_ISDIR(filestat.st_mode))
            {
                print_files_and_sizes_n_r_s(dp->d_name, n, n_val, r, s);
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

void print_files_and_sizes_n_r_s_o(char* dirname, int n, char *n_val, int r, int s, char *out)
{
    //if the function was invoked as a
    char* mode = file_accesed ? "a" : "w";
    file_accesed = 1;
    FILE *output = fopen(out, mode);
    if(NULL == output)
    {
        ERR("fopen");
    }
    //not sure about this if
    if(n && starts_with_string(dirname, n_val))
    {
        return;
    }
    char *starting = NULL;
    if((starting = getcwd(starting, 0)) == NULL)
    {
        ERR("getcwd");
    }
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    struct stat filestat;
    chdir(dirname);
    if(NULL == (dirp = opendir(".")))
    {
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            fclose(output);
            chdir(starting);
            free(starting);
            return;
        }
        else if(ENOENT == errno || ENOTDIR == errno)
        {
            fprintf(stderr, "\"%s\" is not a valid folder\n", dirname);
            fclose(output);
            chdir(starting);
            free(starting);
            return;
        }
        else
        {
            ERR("opendir");
        }
    }
    fprintf(output, "path: %s\n", dirname);
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        //include the -s flag
        if(s && S_ISLNK(filestat.st_mode))
        {
            stat(dp->d_name, &filestat);
        }
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            //include the -n flag
            if(!n || !starts_with_string(dp->d_name, n_val))
            {
                fprintf(output, "%s %lld\n", dp->d_name, filestat.st_size);
            }
        }

    }
    fclose(output);
    //now we perform the recursion on all the subfolders
    if(r)
    {
        rewinddir(dirp);
        while(NULL != (dp = readdir(dirp)))
        {
            if(lstat(dp->d_name, &filestat))
            {
                ERR("lstat");
            }
            //we have ommit . and .. to avoid looping
            if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            {
                continue;
            }
            if(S_ISDIR(filestat.st_mode))
            {
                print_files_and_sizes_n_r_s_o(dp->d_name, n, n_val, r, s, out);
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
