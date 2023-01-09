//started 16:20
//finished 17:14
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "n - number of children processes to create (n > 0)\n");
    exit(EXIT_FAILURE);
}

volatile sig_atomic_t count = 0;

void sig_handler(int sig)
{
    count++;
    printf("[PARENT]: Received %d SIGUSR1 signals\n", count);
}

void sig2_handler(int sig)
{
    printf("[%d]: Received SIGUSR2 - Terminating\n", getpid());
    exit(EXIT_SUCCESS);
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

void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}

void child_work()
{
    srand(getpid());
    int s = 100 + rand() % (200 - 100 + 1);
    //we convert milliseconds to nanoseconds
    struct timespec st = {0 , s * 1000 * 1000};
    while(1)
    {
        nanosleep(&st, NULL);
        kill(getppid(), SIGUSR1);
        //printf("[%d]: Sent %d signals\n", getpid(), 30 - r);
    }
    exit(EXIT_SUCCESS);
}

void parent_work()
{
    //we ignore SIGUSR2 in the parent, so it does not terminate itself
    sethandler(SIG_IGN, SIGUSR2);
    while (count < 100);
    //send SIGUSR2 to all the children
    kill(0, SIGUSR2);
}

int main(int argc, char **argv)
{
    if(argc < 2)
        usage(argv[0]);
    int n = atoi(argv[1]);
    if(n <= 0)
        usage(argv[0]);
    //we set the handler before forking to have it ready
    //the children will start sending signals immediately
    sethandler(sig_handler, SIGUSR1);
    sethandler(sig2_handler, SIGUSR2);
    //not sure if the follwoing line is needed
    //sethandler(sigchld_handler, SIGCHLD);

    pid_t pid;
    while(n-- > 0)
    {
        if((pid = fork()) < 0)
            ERR("fork");
        if(0 == pid)
            child_work();
    }

    parent_work();
    //wait for all the children
    while (wait(NULL) > 0);

    return EXIT_SUCCESS;
}
