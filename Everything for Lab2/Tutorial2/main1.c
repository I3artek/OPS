#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#define ERR(source)\
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s 0<n\n", name);
    exit(EXIT_FAILURE);
}

//#define T1
#define T2

void child_work_t1(int n)
{
    pid_t p = getpid();
    srand(time(NULL) * p);
    sleep(5 + rand() % 6);
    printf("[%d]: Exiting\n", p);
    exit(EXIT_SUCCESS);
}
void create_children(int n, void (*child_work)(int))
{
    for(int i = 0; i < n; i++)
    {
        //we fork n times in parent
        if(0 == fork())
        {
            child_work(i);
        }
    }
}
void wait_and_count(int n)
{
    while (0 != n)
    {
        while (0 < waitpid(0, NULL, WNOHANG))
        {
            n--;
        }
        printf("Alive processes: %d\n", n);
        sleep(3);
    }
}

int wait_no_hang()
{
    errno = 0;
    while (0 < waitpid(0, NULL, WNOHANG))
    {
    }
    return errno == ECHILD ? 1 : 0;
}

void child_work_t2(int n)
{
    pid_t p = getpid();
    srand(time(NULL) * p);
    sleep(5 + rand() % 6);
    printf("[%d]: Exiting\n", p);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{

#ifdef T1
    if(argc < 5)
    {
        usage(argv[0]);
    }
    int n = atoi(argv[1]);
    create_children(n, child_work_t1);
    wait_and_count(n);
    return EXIT_SUCCESS;
#endif

#ifdef T2
    if(argc < 5)
    {
        usage(argv[0]);
    }
    int n = atoi(argv[1]);
    int k = atoi(argv[2]);
    int p = atoi(argv[3]);
    int r = atoi(argv[4]);
    create_children(n, child_work_t2);
    while (1)
    {
        kill((0-getpid()), SIGUSR1);
        sleep(p);
        kill((0-getpid()), SIGUSR2);
        sleep(k);
        if(wait_no_hang())
        {
            exit(EXIT_SUCCESS);
        }
    }
#endif
    //wait_and_count(n);
    return EXIT_SUCCESS;
}
