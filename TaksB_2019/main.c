#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

//macOS - polecam :)
#ifndef SIGRTMAX
#define SIGRTMAX 64
#endif

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage()
{
    fprintf(stderr, "You know how to use it :)\n");
    exit(EXIT_FAILURE);
}

typedef struct{
    pthread_t tid;
    char c;
    int delay;
    int sig_no;
}charGenerator_t;

void ReadArguments(int argc, char **argv, charGenerator_t *thread_infos, int *pattern_size);
void set_thread_info(charGenerator_t *thread_info, char c);
void *thread_work(void *thread_arg);
void sethandler(void (*f)(int), int sig);
void char_handler(int sig);
void process_char(int sig, char *buffer, int size);

volatile sig_atomic_t last_sig;

int main(int argc, char **argv)
{
    int thread_count = (argc - 2) / 2;
    charGenerator_t *thread_infos = calloc(thread_count, sizeof(charGenerator_t));
    int pattern_size;
    ReadArguments(argc, argv, thread_infos, &pattern_size);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMAX - 0);
    sigaddset(&mask, SIGRTMAX - 1);
    sigaddset(&mask, SIGRTMAX - 2);
    sigaddset(&mask, SIGRTMAX - 3);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    sethandler(char_handler, SIGRTMAX - 0);
    sethandler(char_handler, SIGRTMAX - 1);
    sethandler(char_handler, SIGRTMAX - 2);
    sethandler(char_handler, SIGRTMAX - 3);

    for(int i = 0; i < thread_count; i++)
    {
        int err = pthread_create(&(thread_infos[i].tid), NULL, thread_work, &thread_infos[i]);
        if(0 != err)
            ERR("Couldn't create thread");
    }

    char *buffer = calloc(pattern_size, sizeof(char));

    for(int i = 0; i < thread_count; i++)
    {
        int err = pthread_join(thread_infos[i].tid, NULL);
        if(0 != err)
            ERR("Couldn't join thread");
    }

    while(1)
    {
        sigsuspend(&oldmask);
        process_char(last_sig, buffer, pattern_size);
    }

    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    return EXIT_SUCCESS;
}

void ReadArguments(int argc, char **argv, charGenerator_t *thread_infos, int *pattern_size)
{
    int i = 0;
    char c;
    //we use argc - 1 because the last argument is a positional string
    while((c = getopt(argc - 1, argv, "A:T:G:C:")) != -1)
    {
        if('?' == c)
            usage();
        else
        {
            set_thread_info(thread_infos + i, c);
            i++;
        }
    }
    //check for proper number of arguments
    if(argc - 1 != optind)
    {
        usage();
    }
    *pattern_size = 0;
    char *pattern = argv[argc - 1];
    while((c = pattern[(*pattern_size)++]) != '\0')
    {
        if('A' != c && 'T' != c && 'G' != c && 'C' != c)
            usage();
    }
}

void set_thread_info(charGenerator_t *thread_info, char c)
{
    thread_info->c = c;
    errno = 0;
    //optarg is global :)
    thread_info->delay = strtol(optarg, NULL, 0);
    if(errno != 0)
        usage();
    int sig_no = SIGRTMAX;
    if('A' == c)
        sig_no -= 0;
    else if('T' == c)
        sig_no -= 1;
    else if('G' == c)
        sig_no -= 2;
    else if('C' == c)
        sig_no -= 3;
    thread_info->sig_no = sig_no;
}

void *thread_work(void *thread_arg)
{
    charGenerator_t *info = (charGenerator_t *)thread_arg;
    printf("[%d]: %c %d\n", info->tid, info->c, info->delay);
    while(1)
    {
        kill(getpid(), info->sig_no);
    }
    return NULL;
}

void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}

void char_handler(int sig)
{
    last_sig = sig;
}

void process_char(int sig, char *buffer, int size)
{
    char c;
    switch(sig)
    {
        case SIGRTMAX - 0:
            c = 'A';
            break;
        case SIGRTMAX - 1:
            c = 'T';
            break;
        case SIGRTMAX - 2:
            c = 'G';
            break;
        case SIGRTMAX - 3:
            c = 'C';
            break;
        default:
            break;
    }
    for(int i = 0; i < size - 2; i++)
    {
        buffer[i] = buffer[i + 1];
    }
    buffer[size - 2] = c;
    printf("%c", c);
    fflush(stdout);
}