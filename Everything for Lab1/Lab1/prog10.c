#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))

void count(DIR *dirp, struct dirent *dp, char* dirname);

int main(int argc, char **argv)
{
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    for(int i = 1; i< argc; i++)
    {
        count(dirp, dp, argv[i]);
    }
    return EXIT_SUCCESS;
}

void count(DIR *dirp, struct dirent *dp, char* dirname)
{
    struct stat filestat;
    int dirs = 0, files = 0, links = 0, other = 0;
    if(NULL == (dirp = opendir(dirname)))
    {
        ERR("opendir");
    }
    errno = 0;
    while(NULL != (dp = readdir(dirp)))
    {
        if(lstat(dp->d_name, &filestat))
        {
            ERR("lstat");
        }
        if(S_ISDIR(filestat.st_mode))
        {
            dirs++;
        }
        else if(S_ISREG(filestat.st_mode))
        {
            files++;
        }
        else if(S_ISLNK(filestat.st_mode))
        {
            links++;
        }
        else
        {
            other++;
        }
        errno = 0;
    }
    if(errno != 0)
    {
        ERR("readdir");
    }
    if(closedir(dirp))
    {
        ERR("closedir");
    }
    printf("Files: %d, Dirs: %d, Links: %d, Other: %d\n", files, dirs, links, other);
}
