//nice thing: https://gitlab.com/SaQQ/sop1/-/tree/master/
//started 22:40
//finished 00:40
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "n - number of children processes to create (n > 0)\n");
    exit(EXIT_FAILURE);
}

volatile sig_atomic_t count;
volatile sig_atomic_t last_signal;

typedef struct child_info
{
    int pid_no;
    int i;
    int k;
}child_info;

child_info *children;
volatile sig_atomic_t children_exited = 0;

void sig_handler(int sig)
{
    count++;
}


void swap(child_info *a, child_info *b)
{
    int tmp = a->pid_no;
    a->pid_no = b->pid_no;
    b->pid_no = tmp;
    tmp = a->k;
    a->k = b->k;
    b->k = tmp;
    tmp = a->i;
    a->i = b->i;
    b->i = tmp;
}

void QuickSort(child_info A[], int size)
{
    if(size <= 1)
    {
        return;
    }
    child_info *pivot = A + size - 1;
    child_info *l = A;
    child_info *r = A + size - 2;
    while(r + 1 != l)
    {
        if(r->k <= pivot->k && l->k > pivot->k)
        {
            swap(r, l);
            l++;
            r--;
        }
        if(r->k > pivot->k)
        {
            r--;
        }
        if(l->k <= pivot->k)
        {
            l++;
        }
    }
    if(pivot != l)
    {
        swap(pivot, l);
    }
    QuickSort(A, r - A + 1);
    QuickSort(l + 1, size + A - r - 2);
}

void sigchld_handler(int sig, siginfo_t *info, void* uap)
{
    last_signal = sig;
    children[children_exited].pid_no = info->si_pid;
    children[children_exited].k = info->si_status % 11;
    children[children_exited].i = info->si_status / 11;
    children_exited++;
}

void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}

void sethandler_advanced(void (*f)(int, siginfo_t*, void*), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_sigaction = f;
    sa.sa_flags = SA_SIGINFO;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}

void child_work()
{
    srand(getpid());
    int k = rand() % (10 - 0 + 1);
    printf("[%d]: k = %d\n", getpid(), k);
    //full time for sleep one second
    struct timespec full = { 1, 0};
    struct timespec t;
    for(int s = 0; s < 10; s++)
    {
        if(s < k)
            kill(0, SIGUSR1);
        //set sleep time to one second
        t = full;
        //sleep until one second elapses
        //pick up where it left at signal interruption
        while (nanosleep(&t, &t) != 0);
    }
    printf("[%d]: k = %d i = %d\n", getpid(), k, count);
    exit(count * 11 + k);
}

void parent_work(int n, sigset_t oldmask)
{
    children = calloc(n, sizeof (child_info));
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
    QuickSort(children, n);

    int fd;
    if((fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
        ERR("open");

    char *line = calloc(40, sizeof (char));

    for(int i = 0; i < children_exited; i++)
    {
        sprintf(line, "PID: %d k: %d i: %d\n", children[i].pid_no, children[i].k, children[i].i);
        //printf("[PARENT]: PID: %d k = %d i = %d\n", children[i].pid_no, children[i].k, children[i].i);
        int j = 0;
        while(line[j] != '\0')
        {
            if(write(fd, line + j, 1) < 0)
                ERR("write");
            j++;
        }
    }
    close(fd);

    free(children);
}

int main(int argc, char **argv)
{
    if(argc != 2)
        usage(argv[0]);
    int n = atoi(argv[1]);
    if(n < 1)
        usage(argv[0]);

    sethandler(sig_handler, SIGUSR1);
    sethandler_advanced(sigchld_handler, SIGCHLD);

    sigset_t mask, oldmask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    pid_t pid;
    //create n children
    for(int i = 0; i < n; i++)
    {
        if((pid = fork()) < 0)
            ERR("fork");
        if(0 == pid)
            child_work();
    }

    parent_work(n, oldmask);

    //we set this, so the SIGUSR1 does not interrupt the wait() call
    sethandler(SIG_IGN, SIGUSR1);

    while (wait(NULL) > 0);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    return EXIT_SUCCESS;
}
