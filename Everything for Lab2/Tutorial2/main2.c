#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define ERR(source)\
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s 0<n\n", name);
    exit(EXIT_FAILURE);
}

volatile sig_atomic_t sigusr_n = 0;

void handler(int sig)
{
    sigusr_n = sig;
}

//Seems to be very useful as a to-go function
void sethandler( void (*f)(int), int sigNo) {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sigchld_handler(int sig)
{
    pid_t pid;
    for(;;)//while(1) works the same - there is virtually no difference after compilation
    {
        //wait for child process (if there is one)
        pid = waitpid(0, NULL, WNOHANG);
        //waitpid return the id of the waited child process
        //0 is returned when no childs are to be waited (with WNOHANG)
        if(pid == 0)
            return;
        //-1 indicates an error
        if(pid <= 0)
        {
            //ECHILD means there are no children left
            if(ECHILD == errno)
                return;
            ERR("waitpid");
        }
    }
}

void child_work(int n, int r)
{
    pid_t p = getpid();
    srand(time(NULL) * p);
    int t = 5 + rand() % (10 - 5 + 1);
    int t_left;
//    for(int i = 0; i < r; i++)
//    {
//        sleep(t);
//        printf("[%d]: ", p);
//        printf( SIGUSR1 == sigusr_n ? "SUCCESS\n" : SIGUSR2 == sigusr_n ? "FAILURE\n" : "");
//    }
    while (r-- > 0)
    {
        for(t_left = t; t_left > 0; t_left = sleep(t_left));
        printf("[%d]: ", p);
        printf( SIGUSR1 == sigusr_n ? "SUCCESS\n" : SIGUSR2 == sigusr_n ? "FAILURE\n" : "");
    }
    //printf("[%d]: Exiting\n", p);
    exit(EXIT_SUCCESS);
}

void create_children(int n, int r)
{
    for(int i = 0; i < n; i++)
    {
        //we fork n times in parent
        if(0 == fork())
        {
            sethandler(handler, SIGUSR1);
            sethandler(handler, SIGUSR2);
            child_work(i, r);
        }
    }
}

int main(int argc, char **argv)
{
    if(argc < 5)
    {
        usage(argv[0]);
    }

    sethandler(sigchld_handler, SIGCHLD);
    sethandler(SIG_IGN, SIGUSR1);
    sethandler(SIG_IGN, SIGUSR2);

    int group_pid = 0 - getpid();

    int n = atoi(argv[1]);
    int k = atoi(argv[2]);
    int p = atoi(argv[3]);
    int r = atoi(argv[4]);
    create_children(n, r);
    for(int i = 0; i < r; i++)//while (1)
    {
        kill(group_pid, SIGUSR1);
        sleep(p);
        kill(group_pid, SIGUSR2);
        sleep(k);
    }
    while (wait(NULL) > 0);
    return EXIT_SUCCESS;
}
