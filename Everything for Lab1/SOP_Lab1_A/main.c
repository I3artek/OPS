#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ftw.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s (folderName size) ... \n", pname);
    exit(EXIT_FAILURE);
}

int has_extension(const char *filename, const char *ext)
{
    if('.' != filename[strlen(filename) - strlen(ext) - 1])
    {
        return 0;
    }
    return !strcmp(filename + strlen(filename) - strlen(ext), ext);
}

int check_file(const char *pathname, const struct stat *filestat, int type, struct FTW *val);

int c_count = 0;
int h_count = 0;
int txt_count = 0;
char *ext_ommit = "f";
char *temp_file;
char *o = NULL;

void count_files(char *dirname);
void print_analysis(char *dirname);

int main(int argc, char **argv)
{
    //parse options to set proper flags
    char c;
    while((c = getopt(argc, argv, "p:e:o")) != -1)
    {
        if('?' == c)
        {
            usage(argv[0]);
        }
        else if('o' == c)
        {
            o = ".analysis";
        }
    }
    //check for proper number of arguments
    if(argc > optind)
    {
        usage(argv[0]);
    }
    optind = 1;
    while((c = getopt(argc, argv, "p:e:o")) != -1)
    {
        if('p' == c)
        {
            temp_file = optarg;
            if('e' == (c = getopt(argc, argv, "p:e:o")))
            {
                ext_ommit = optarg;
            }
            else if(-1 != c)
            {
                optind -= 2;
            }
            nftw(temp_file, check_file, 20, FTW_PHYS);
            print_analysis(temp_file);
        }
    }

    return EXIT_SUCCESS;
}

void print_analysis(char *dirname)
{
    FILE *out = stdout;
    if(NULL != o)
    {
        char *starting = NULL;
        if((starting = getcwd(starting, 0)) == NULL)
        {
            ERR("getcwd");
        }
        if(chdir(dirname))
        {
            ERR("chdir");
        }
        out = fopen(o, "w");
        if(NULL == out)
        {
            ERR("fopen");
        }
        chdir(starting);
        free(starting);
    }
    fprintf(out, "Directory: %s\n", dirname);
    fprintf(out, "Directory analysis:\n");
    fprintf(out, "txt: %d\n", txt_count);
    fprintf(out, "c: %d\n", c_count);
    fprintf(out, "h: %d\n", h_count);

    txt_count = 0;
    c_count = 0;
    h_count = 0;
    ext_ommit = "f";

    if(stdout != out)
    {
        fclose(out);
    }
}

int check_file(const char *pathname, const struct stat *filestat, int type, struct FTW *val)
{
    ext_ommit = NULL == ext_ommit ? "f" : ext_ommit;
    if(FTW_F == type)
    {
        if(has_extension(pathname, "c") && strcmp("c", ext_ommit))
        {
            c_count++;
        }
        else if(has_extension(pathname, "h") && strcmp("h", ext_ommit))
        {
            h_count++;
        }
        else if(has_extension(pathname, "txt") && strcmp("txt", ext_ommit))
        {
            txt_count++;
        }
    }
    return 0;
}
