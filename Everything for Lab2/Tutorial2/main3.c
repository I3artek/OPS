#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))


//variable to keep track of the last signal
volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t counter2 = 0;

//function to set the desired handler for chosen signals
void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}

//custom handler that changes last_signal to the value of last signal
void sig_handler(int sig)
{
    last_signal = sig;
    if(SIGUSR2 == sig)
        counter2++;
}

//handler that waits for children
void sigchld_handler(int sig)
{
    pid_t pid;
    while (1)
    {
        pid = waitpid(0, NULL, WNOHANG);
        if(0 == pid)
            return;
        if(pid <= 0)
        {
            if(ECHILD == errno)
                return;
            ERR("waitpid");
        }
    }
}

void child_work(int m, int n)
{
    int count = 0;
    struct timespec t = { 0, m * 1000};
    pid_t ppid = getppid();
    while(1)
    {
        for(int i = 0; i < n; i++)
        {
            nanosleep(&t, NULL);
            if(kill(ppid, SIGUSR1))
                ERR("kill");
        }
        nanosleep(&t, NULL);
        if(kill(ppid, SIGUSR2))
            ERR("kill");
        count++;
        printf("[%d] sent %d SIGUSR2\n", getpid(), count);
    }
}

void parent_work(sigset_t oldmask)
{
    int count = 0;
    while(1)
    {
        last_signal = 0;
        while(SIGUSR2 != last_signal)
            sigsuspend(&oldmask);
        count++;
        printf("[PARENT] received %d SIGUSR2\n", counter2);
    }
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s m  p\n", name);
    fprintf(stderr, "m - number of 1/1000 milliseconds between signals [1,999], "
                    "i.e. one milisecond maximum\n");
    fprintf(stderr, "p - after p SIGUSR1 send one SIGUSER2  [1,999]\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if(3 != argc)
        usage(argv[0]);
    int m, n;
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    if(m <= 0 || m > 999 || n <= 0 || n > 999)
        usage(argv[0]);

    sethandler(sigchld_handler, SIGCHLD);
    sethandler(sig_handler, SIGUSR1);
    sethandler(sig_handler, SIGUSR2);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    pid_t pid;
    if((pid = fork()) < 0)
        ERR("fork");
    if( 0 == pid)
        child_work(m, n);
    else
        parent_work(oldmask);

    while (wait(NULL) > 0);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    return EXIT_SUCCESS;
}
