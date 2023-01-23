#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s N M T\n", name);
    exit(EXIT_FAILURE);
}

#define N_MIN 5
#define N_MAX 500
#define M_MIN 3
#define M_MAX 300
#define T_MIN 50
#define THREAD_SLEEP_MIN 10
#define THREAD_SLEEP_MAX 1000
#define MILLI_TO_NANO (1000 * 1000)
#define CANCEL_TIME 500

typedef struct{
    int N;
    int M;
    int T;
    int *circle;
}shared_values;

typedef struct{
    pthread_t tid;
    shared_values *v;
    int cell;
}thread_arg;

void create_threads(shared_values *v, thread_arg *args);
void *tulkun_work(void *arg_);
void ProcessArguments(int argc, char **argv, shared_values *v);
void join_threads(thread_arg *args, int M);
void cancel_threads(thread_arg *args, int M);
void print_array(int *circle, int size);

int main(int argc, char **argv)
{
    srand(getpid());
    shared_values V = {0,0,0, 0};
    ProcessArguments(argc, argv, &V);
    thread_arg *args = (thread_arg *)calloc(V.M, sizeof(thread_arg));
    if(!args)
        ERR("calloc");
    V.circle = (int *) calloc(V.N, sizeof(int));
    if(!V.circle)
        ERR("calloc");
    create_threads(&V, args);
    struct timespec st_full = {1, 0};
    int time_slept = 0;
    while(time_slept < V.T)
    {
        struct timespec st = st_full;
        while(nanosleep(&st, &st) != 0);
        time_slept++;
        print_array(V.circle, V.N);
    }
    cancel_threads(args, V.M);
    join_threads(args, V.M);
    free(args);
    free(V.circle);
    return EXIT_SUCCESS;
}

void *tulkun_work(void *arg_)
{
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    thread_arg *arg = (thread_arg *)arg_;
    unsigned int seed = arg->cell;
    int direction = rand_r(&seed) % 2;
    //0 - left 1 - right
    struct timespec st = { 1, 0 };
    while(1)
    {
        nanosleep(&st, &st);
        sleep(1);
        int next_cell = direction ? arg->cell + 1 : arg->cell - 1;
        if(next_cell == arg->v->N)
            next_cell = 0;
        if(next_cell < 0)
            next_cell = arg->v->N - 1;
        if(arg->v->circle[next_cell] == 0)
        {
            arg->v->circle[next_cell] = 1;
            arg->v->circle[arg->cell] = 0;
            arg->cell = next_cell;
        }
        else
        {
            direction = !direction;
        }
    }
    //int delay = THREAD_SLEEP_MIN + rand_r(&r) % (THREAD_SLEEP_MAX - THREAD_SLEEP_MIN + 1);
    //struct timespec st = { 1, MILLI_TO_NANO };
    //nanosleep(&st, &st);
    printf("*\n");
    return NULL;
}

void ProcessArguments(int argc, char **argv, shared_values *v)
{
    if(argc != 4)
        usage(argv[0]);
    //atoi returns 0 on error
    //0 is outside our bounds, so we can use it safely
    //no need for strtol
    v->N = atoi(argv[1]);
    if(v->N < N_MIN || v->N > N_MAX)
        usage(argv[0]);

    v->M = atoi(argv[2]);
    if(v->M < M_MIN || v->N > M_MAX)
        usage(argv[0]);

    if(v->M > v->N)
        usage(argv[0]);

    v->T = atoi(argv[3]);
    if(v->T < T_MIN)
        usage(argv[0]);
}

void create_threads(shared_values *v, thread_arg *args)
{
    for(int i = 0; i < v->M; i++)
    {
        args[i].v = v;
        while(1)
        {
            //we can use rand() here - before threads are spawned
            int r = rand() % v->N;
            if(v->circle[r] == 0)
            {
                v->circle[r] = 1;
                args[i].cell = r;
                break;
            }
        }
    }
    for(int i = 0; i < v->M; i++)
    {
        int err = pthread_create(&args[i].tid, NULL, tulkun_work, &args[i]);
        if(err != 0)
            ERR("Couldn't create thread");
    }
}

void join_threads(thread_arg *args, int M)
{
    for(int i = 0; i < M; i++)
    {
        int err = pthread_join(args[i].tid, NULL);
        if(0 != err)
            ERR("Couldn't join thread");
    }
}

void cancel_threads(thread_arg *args, int M)
{
    for(int i = 0; i < M; i++)
    {
        int err = pthread_cancel(args[i].tid);
        if(0 != err)
            ERR("Couldn't join thread");
    }
}

void print_array(int *circle, int size)
{
    printf("[ ");
    for(int i = 0; i < size; i++)
    {
        printf("%d ", circle[i]);
    }
    printf("]\n");
}
