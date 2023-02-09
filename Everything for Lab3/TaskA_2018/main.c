#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define MILLITONANO (1000 * 1000)
#define SLEEP_TIME 500 * MILLITONANO
#define THREAD_SLEEP 1
#define V_MIN 1
#define V_MAX 255




//FUNCTIONS SHOULD BE SHORTER -- CHECK WEBSITE FOR CLEAN CODE REQUIREMENTS




#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n t\n", name);
    exit(EXIT_FAILURE);
}

typedef struct{
    int *vector;
    int vector_size;
    int GUESSED_VALUE;
    pthread_mutex_t vector_mutex;
    pthread_mutex_t g_value_mutex;
}shared_v;

typedef struct{
    pthread_t tid;
    shared_v *shared_values;
    unsigned int seed;
    int cancelled;
}thread_arg_t;

volatile sig_atomic_t sigint_count = 0;
volatile sig_atomic_t sigusr2_count = 0;
volatile sig_atomic_t quit = 0;

void process_input(int argc, char **argv, int *n, int *t);

void init_shared_values(shared_v *shared_values, int n);

void *thread_work(void *thread_arg);

void create_threads(void * (*thread_work)(void *), thread_arg_t *thread_args, int t);

void join_threads(thread_arg_t *thread_args, int t);

void init_thread_args(thread_arg_t *thread_args, shared_v *shared_values, int t);

void unlock_mutex(void *mutex_pointer);

void sethandler(void (*f)(int), int sig);

void print_vector(shared_v shared_values);

void sigint_handler(int sig);

void sigusr2_handler(int sig);

void handle_signals_synchronous(shared_v shared_values, thread_arg_t *thread_args, int t);

void sigquit_handler(int sig);

int main(int argc, char **argv)
{
    int n, t;
    process_input(argc, argv, &n, &t);
    shared_v shared_values;
    init_shared_values(&shared_values, n);
    thread_arg_t *thread_args = (thread_arg_t *)calloc(t, sizeof(thread_arg_t));
    if(thread_args == NULL)
    {
        ERR("calloc");
    }
    init_thread_args(thread_args, &shared_values, t);
    create_threads(thread_work, thread_args, t);\

    struct timespec ts_base = { 0, SLEEP_TIME };
    struct timespec ts;

    sethandler(sigint_handler, SIGINT);
    sethandler(sigusr2_handler, SIGUSR2);
    sethandler(sigquit_handler, SIGQUIT);

    while(!quit)
    {
        ts = ts_base;
        while(nanosleep(&ts, NULL) != 0)
        {
            //this gets executed only when it was interrupted by a signal
            handle_signals_synchronous(shared_values, thread_args, t);
        }

        if(pthread_mutex_lock(&shared_values.vector_mutex) != 0)
        {
            ERR("pthread_mutex_lock");
        }
        print_vector(shared_values);
        while(sigint_count > 0)
        {
            int index = rand() % shared_values.vector_size;
            shared_values.vector[index] = V_MIN + (rand() % (V_MAX - V_MIN + 1));
            sigint_count--;
        }
        if(pthread_mutex_unlock(&shared_values.vector_mutex) != 0)
        {
            ERR("pthread_mutex_lock");
        }
        handle_signals_synchronous(shared_values, thread_args, t);
    }

    for(int i = 0; i < t; i++)
    {
        if(thread_args[i].cancelled == 0)
        {
            if(pthread_cancel(thread_args[i].tid) != 0)
            {
                ERR("pthread_cancel");
            }
            thread_args[i].cancelled = 1;
        }
    }

    //join_threads(thread_args, t);
    free(shared_values.vector);
    free(thread_args);
    return EXIT_SUCCESS;
}

void create_threads(void * (*thread_work)(void *), thread_arg_t *thread_args, int t)
{
    for(int i = 0; i < t; i++)
    {
        int err = pthread_create(&thread_args[i].tid, NULL, thread_work, &thread_args[i]);
        if(err != 0)
            ERR("Couldn't create thread");
    }
}

void process_input(int argc, char **argv, int *n, int *t)
{
    if(argc != 3)
    {
        usage(argv[0]);
    }

    //We can use atoi() as it returns 0 on error
    //And if we get 0 from it, we don't care if this was user
    //input or an error in parsing, as we need n,t > 0
    *n = atoi(argv[1]);
    *t = atoi(argv[2]);
    if(*n == 0 || *t == 0)
    {
        usage(argv[0]);
    }
}

void init_shared_values(shared_v *shared_values, int n)
{
    //calloc initializes the memory with 0s
    shared_values->vector = (int *)calloc(n, sizeof(int));
    if(shared_values->vector == NULL)
    {
        ERR("calloc");
    }
    shared_values->vector_size = n;
    if(pthread_mutex_init(&shared_values->vector_mutex, NULL) != 0)
    {
        ERR("pthread_mutex_init");
    }
    if(pthread_mutex_init(&shared_values->g_value_mutex, NULL) != 0)
    {
        ERR("pthread_mutex_init");
    }
    shared_values->GUESSED_VALUE = 0;
}

void *thread_work(void *thread_arg)
{
    thread_arg_t *arg = (thread_arg_t *)thread_arg;
    printf("[%d]\n", pthread_self());
    int index;
    int val;
    while (1)
    {
        index = rand_r(&arg->seed) % arg->shared_values->vector_size;
        if(pthread_mutex_lock(&arg->shared_values->vector_mutex) != 0)
        {
            ERR("pthread_mutex_lock");
        }
        pthread_cleanup_push(unlock_mutex, &arg->shared_values->vector_mutex);
        val = arg->shared_values->vector[index];
        pthread_cleanup_pop(1);
        if(val == 0)
        {
            sleep(THREAD_SLEEP);
            continue;
        }
        if(pthread_mutex_lock(&arg->shared_values->g_value_mutex) != 0)
        {
            ERR("pthread_mutex_lock");
        }
        pthread_cleanup_push(unlock_mutex, &arg->shared_values->g_value_mutex);
        if(val == arg->shared_values->GUESSED_VALUE)
        {
            if(kill(getpid(), SIGUSR2) != 0)
            {
                ERR("kill");
            }
        }
        else if(val > 0)
        {
            arg->shared_values->GUESSED_VALUE = val;
            //printf("[%d]: setting GUESSED_VALUE to %d\n", pthread_self(), val);
        }
        pthread_cleanup_pop(1);
        sleep(THREAD_SLEEP);
    }
    return NULL;
}

void join_threads(thread_arg_t *thread_args, int t)
{
    for(int i = 0; i < t; i++)
    {
        int err = pthread_cancel(thread_args[i].tid);
        if(0 != err)
            ERR("Couldn't cancel thread");
        err = pthread_join(thread_args[i].tid, NULL);
        if(0 != err)
            ERR("Couldn't join thread");
    }
}

void init_thread_args(thread_arg_t *thread_args, shared_v *shared_values, int t)
{
    //can be changed to srand(getpid())
    srand(1);
    for(int i = 0; i < t; i++)
    {
        thread_args[i].shared_values = shared_values;
        thread_args[i].seed = rand();
        thread_args[i].cancelled = 0;
    }
}

void unlock_mutex(void *mutex_pointer)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)mutex_pointer;
    if(pthread_mutex_unlock(mutex) != 0)
    {
        ERR("pthread_mutex_unlock");
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

void print_vector(shared_v shared_values)
{
    printf("[ ");
    for(int i = 0; i < shared_values.vector_size; i++)
    {
        printf("%d ", shared_values.vector[i]);
    }
    printf("]\n");
}

void sigint_handler(int sig)
{
    sigint_count++;
}

void sigusr2_handler(int sig)
{
    sigusr2_count++;
}

void handle_signals_synchronous(shared_v shared_values, thread_arg_t *thread_args, int t)
{
    while(sigint_count > 0)
    {
        if(pthread_mutex_lock(&shared_values.vector_mutex) != 0)
        {
            ERR("pthread_mutex_lock");
        }
        int index = rand() % shared_values.vector_size;
        shared_values.vector[index] = V_MIN + (rand() % (V_MAX - V_MIN + 1));
        if(pthread_mutex_unlock(&shared_values.vector_mutex) != 0)
        {
            ERR("pthread_mutex_unlock");
        }
        sigint_count--;
    }
    while(sigusr2_count > 0)
    {
        if(pthread_mutex_lock(&shared_values.g_value_mutex) != 0)
        {
            ERR("pthread_mutex_lock");
        }
        printf("GUESSED VALUE: %d\n", shared_values.GUESSED_VALUE);
        if(pthread_mutex_unlock(&shared_values.g_value_mutex) != 0)
        {
            ERR("pthread_mutex_lock");
        }
        int i;
        //find a thread that wasn't already canceled
        do {
            i = rand() % t;
        } while(thread_args[i].cancelled != 0);
        if(pthread_cancel(thread_args[i].tid) != 0)
        {
            ERR("pthread_cancel");
        }
        thread_args[i].cancelled = 1;
        printf("Canceling thread: %d\n", thread_args[i].tid);
        sigusr2_count--;
    }
}

void sigquit_handler(int sig)
{
    quit = 1;
}
