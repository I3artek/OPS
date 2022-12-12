#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s (folderName size) ... \n", pname);
    exit(EXIT_FAILURE);
}

void print_files(char* dirname);
void print_file_sizes(char* dirname);
int size_of_files(char* dirname);

int main(int argc, char **argv)
{
    //part 1.
    //print_files(".");
    //part 2.
    //print_file_sizes(".");
    //part 3.
    /*
    for(int i = 1; i < argc; i += 2)
    {
        print_file_sizes(argv[i]);
    }
     */
    //part 4.
    /*
    for(int i = 1; i < argc; i++)
    {
        int size = size_of_files(argv[i]);
        if(-1 == size)
        {
            i++;
            continue;
        }
        if(i + 1 == argc)
        {
            usage(argv[0]);
        }
        int limit = atoi(argv[++i]);
        if(size > limit)
        {
            printf("%s\n", argv[i - 1]);
        }
    }
     */
    //part 5.
    FILE *output = fopen("out.txt", "a");
    if(NULL == output)
    {
        ERR("fopen");
    }
    for(int i = 1; i < argc; i++)
    {
        int size = size_of_files(argv[i]);
        if(-1 == size)
        {
            i++;
            continue;
        }
        if(i + 1 == argc)
        {
            usage(argv[0]);
        }
        int limit = atoi(argv[++i]);
        if(size > limit)
        {
            fprintf(output, "%s\n", argv[i - 1]);
        }
    }
    fclose(output);
    return EXIT_SUCCESS;
}

void print_files(char* dirname)
{
    char *starting = NULL;
    if(getcwd(starting, 0) == NULL)
    {
        ERR("getcwd");
    }
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    if(NULL == (dirp = opendir(dirname)))
    {
        ERR("opendir");
    }
    while(NULL != (dp = readdir(dirp)))
    {
        printf("%s\n", dp->d_name);
    }
    if(closedir(dirp))
    {
        ERR("closedir");
    }
    chdir(starting);
    free(starting);
}

void print_file_sizes(char* dirname)
{
    char *starting = NULL;
    if(getcwd(starting, 0) == NULL)
    {
        ERR("getcwd");
    }
    chdir(dirname);
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    struct stat filestat;
    long long sum = 0;
    if(NULL == (dirp = opendir(dirname)))
    {
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            return;
        }
        else if(ENOENT == errno || ENOTDIR == errno)
        {
            fprintf(stderr, "\"%s\" is not a valid folder\n", dirname);
            return;
        }
        else
        {
            ERR("opendir");
        }
    }
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        sum += filestat.st_size;
        printf("%lld\n", filestat.st_size);
    }
    printf("Sum: %lld\n", sum);
    if(closedir(dirp))
    {
        ERR("closedir");
    }
    chdir(starting);
    free(starting);
}

int size_of_files(char* dirname)
{
    char *starting = NULL;
    if(getcwd(starting, 0) == NULL)
    {
        ERR("getcwd");
    }
    chdir(dirname);
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    struct stat filestat;
    long long sum = 0;
    errno = 0;
    if(NULL == (dirp = opendir(dirname)))
    {
        if(EACCES == errno)
        {
            fprintf( stderr, "No access to folder \"%s\"\n", dirname);
            return -1;
        }
        else if(ENOENT == errno || ENOTDIR == errno)
        {
            fprintf(stderr, "\"%s\" is not a valid folder\n", dirname);
            return -1;
        }
        else
        {
            ERR("opendir");
        }
    }
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        sum += filestat.st_size;
    }
    if(closedir(dirp))
    {
        ERR("closedir");
    }
    chdir(starting);
    free(starting);
    return sum;
}
