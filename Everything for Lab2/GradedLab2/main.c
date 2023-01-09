#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s s s s ...\n", name);
    fprintf(stderr, "s - digit 1 or 2, repeated n times ( n <= 8)\n");
    exit(EXIT_FAILURE);
}

volatile sig_atomic_t sig1count = 0;
volatile sig_atomic_t sig2count = 0;
volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t c1_count = 0;
volatile sig_atomic_t c2_count = 0;

typedef struct child_info
{
    pid_t pid;
    int sig;
}child_info;

//hate to do it as global variable, but cannot do otherwise :(
child_info *children;
int children_size = 0;

void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}
void sethandler_info(void (*f)(int, siginfo_t*, void*), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_sigaction = f;
    sa.sa_flags = SA_SIGINFO;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}


void sig1_handler(int sig)
{
    sig1count++;
    //printf("*\n");
}

void sig2_handler(int sig)
{
    sig2count++;
    //printf("*\n");
}

int which_signal(pid_t p)
{
    for(int i = 0; i < children_size; i++)
    {
        if(children[i].pid == p)
            return children[i].sig;
    }
    return -1;
}

void sigchld_handler(int sig, siginfo_t *info, void* uap)
{
    last_signal = sig;
    int sig_sent_by_that_child = which_signal(info->si_pid);
    if(sig_sent_by_that_child == SIGUSR1)
        c1_count += info->si_status;
    else
        c2_count += info->si_status;
}

void child_work(int i, int s)
{
    srand(getpid() * i);
    int c = rand() % (i + 1);
    int t = 10 + rand() % (30 - 10 + 1);
    printf("PID: %d, i: %d, c: %d, s: %d, t: %d\n", getpid(), i, c, s, t);

    //sending signals
    int sig_to_send = s == 1 ? SIGUSR1 : SIGUSR2;
    pid_t ppid = getppid();
    //convert milliseconds to nanoseconds
    struct timespec ts = { 0, t * 1000 * 1000};
    for(int j = 0; j < c; j++)
    {
        kill(ppid, sig_to_send);
        nanosleep(&ts, NULL);
    }
    //change exit number
    free(children);
    exit(c);
}

void parent_work(sigset_t oldmask, int n)
{
    for(int i = 0; i < n; i++)
    {
        last_signal = 0;
        while (SIGCHLD != last_signal)
        {
            //let one signal from blocked ones in until it is SIGCHLD
            sigsuspend(&oldmask);
        }
        //when it was SIGCHLD go to next iteration
    }
    printf("SIGUSR1: %d, SIGUSR2: %d, C1: %d, C2: %d\n", sig1count, sig2count, c1_count, c2_count);

    //saving to file
    int fd;
    if((fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
        ERR("open");
    char *line = calloc(100, sizeof (char));
    sprintf(line ,"SIGUSR1: %d, SIGUSR2: %d, C1: %d, C2: %d\n", sig1count, sig2count, c1_count, c2_count);
    if(write(fd, line, strlen(line)) < 0)
        ERR("write");
    close(fd);
    free(line);
}

int main(int argc, char **argv)
{
    if(argc > 9)
        usage(argv[0]);
    int s;
    //check for errors before creating children
    for(int i = 1; i < argc; i++)
    {
        s = atoi(argv[i]);
        if(s != 1 && s != 2)
            usage(argv[0]);
    }

    children_size = argc - 1;

    //set the handlers before creating children so they are ready
    sethandler(sig1_handler, SIGUSR1);
    sethandler(sig2_handler, SIGUSR2);
    sethandler_info(sigchld_handler, SIGCHLD);

    sigset_t mask, oldmask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);


    children = (child_info*) calloc(argc - 1, sizeof (child_info));
    //create children
    pid_t pid;
    for(int i = 1; i < argc; i++)
    {
        s = atoi(argv[i]);
        if((pid = fork()) < 0)
            ERR("fork");
        if(0 == pid)
            child_work(i, s);
        children[i - 1].pid = pid;
        children[i - 1].sig = s == 1 ? SIGUSR1 : SIGUSR2;
    }

    parent_work(oldmask, argc - 1);

    while (wait(NULL) > 0);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    free(children);

    return EXIT_SUCCESS;
}
