//nice thing: https://gitlab.com/SaQQ/sop1/-/tree/master/
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
    fprintf(stderr, "USAGE: %s n m\n", name);
    fprintf(stderr, "n - number of children processes to create (3 <= n <= 10)\n");
    fprintf(stderr, "m - pause in miliseconds (1 <= m <= 500)\n");
    exit(EXIT_FAILURE);
}

volatile sig_atomic_t sig1_count = 0;
volatile sig_atomic_t count = 0;
volatile sig_atomic_t last_signal = 0;

void sig1_handler(int sig)
{
    sig1_count++;
}

void sig2_handler(int sig)
{
    printf("[%d]: Received %d SIGUSR1\n", getpid(), sig1_count);
    count += sig1_count * (rand() % 5 - 2);
    printf("[%d]: Counter: %d\n", getpid(), count);
    sig1_count = 0;
}

void sigterm_handler(int sig)
{
    last_signal = sig;
}

void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}

void child_work(int i)
{
    //add reading from file etc
    int fd;
    //allocate for "state.XX" + null terminating character
    char *filename = calloc(9, sizeof (char));
    sprintf(filename, "state.%d", i);
    //O_RDWR - read write
    //O_CREAT - create if not exists
    //0777 - let umask do its job
    if((fd = open(filename, O_RDWR | O_CREAT, 0777)) < 0)
        ERR("open");
    free(filename);

    int i_count = 0;
    if(read(fd, &i_count, sizeof (int)) == sizeof (int))
    {
        count = i_count;
    }

    printf("[%d]: %d Counter: %d\n", getpid(), i, count);

    srand(getpid() * i);
    int timeout = 2 + rand() % (10 - 2 + 1);
    //when a signal is delivered it interrupts sleep and sleep() > 0
    //when the time elapses without a signal sleep returns 0
    while (sleep(timeout) != 0)
    {
        if(SIGTERM == last_signal)
        {
            i_count = count;
            printf("[%d]: Exiting with counter: %d\n", getpid(), count);
            if(lseek(fd, 0, SEEK_SET) < 0)
                ERR("lseek");
            if(write(fd, &i_count, sizeof (int)) < 0)
                ERR("write");
            close(fd);
            exit(EXIT_SUCCESS);
        }
    }
    printf("[%d]: Exiting with counter: %d\n", getpid(), count);
    exit(EXIT_SUCCESS);
}

void parent_work(int m)
{
    sethandler(SIG_IGN, SIGTERM);
    sethandler(SIG_IGN, SIGUSR1);
    sethandler(SIG_IGN, SIGUSR2);
    //convert milliseconds to nanoseconds
    struct timespec st = { 0, m * 1000 * 1000};
    int k;
    char *buf = calloc(6, sizeof (char));
    while (scanf("%5s", buf) == 1)
    {
        k = atoi(buf);
        //when k is out of range or there was not an integer
        if(k < 2 || k > 20)
        {
            if(strcmp(buf, "exit") == 0)
            {
                kill(0, SIGTERM);
                free(buf);
                return;
            }
            else
            {
                fprintf(stderr, "Type number 2 <= k <= 20 or \"exit\"\n");
            }
        }
        else
        {
            while (k-- > 0)
            {
                kill(0, SIGUSR1);
                nanosleep(&st, NULL);
            }
            kill(0, SIGUSR2);
        }
    }
    ERR("scanf");
}

int main(int argc, char **argv)
{
    if(argc < 3)
        usage(argv[0]);
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    if(n < 3 || n > 10 || m < 1 || m > 500)
        usage(argv[0]);

    sethandler(sig1_handler, SIGUSR1);
    sethandler(sig2_handler, SIGUSR2);
    sethandler(sigterm_handler, SIGTERM);

    pid_t pid;
    for(int i = 1; i <= n; i++)
    {
        if((pid = fork()) < 0)
            ERR("fork");
        if(0 == pid)
            child_work(i);
    }

    parent_work(m);
    //while (1);

    while (wait(NULL) > 0);

    return EXIT_SUCCESS;
}
