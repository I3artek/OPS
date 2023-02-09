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
    fprintf(stderr, "USAGE: %s N T\n", name);
    exit(EXIT_FAILURE);
}

#define HOUSE_MAX 100
#define HOUSE_MIN 1
#define SLEEP_MAX 50
#define SLEEP_MIN 10
#define BLOW_INTERVAL_MAX 500
#define BLOW_INTERVAL_MIN 100
#define BLOW_B_MIN 1

#define MILLITONANO (1000 * 1000)

typedef struct{
    pthread_t tid;
    int *house_pointer;
    pthread_mutex_t mutex;
}pig_info;

void *pig_work(void *pig_arg);
void print_array(int *arr, int size);
int blow_strength(int *houses, int size, unsigned int *seed);
void wolf_blow(int *houses, int size, pig_info *pigs, unsigned int *seed);
void sethandler(void (*f)(int), int sig);


int main(int argc, char **argv)
{
    if(argc < 3)
        usage(argv[0]);
    int N = atoi(argv[1]);
    if(N <= 0)
        usage(argv[0]);
    int T = atoi(argv[2]);
    if(T <= 0)
        usage(argv[0]);


    //array with args for threads
    pig_info *pigs = (pig_info *) calloc(N, sizeof(pig_info));

    srand(getpid());

    //interval for wolf - must be here - uses rand()
    int blow_interval = BLOW_INTERVAL_MIN + rand() % (BLOW_INTERVAL_MAX - BLOW_INTERVAL_MIN + 1);

    int *houses = (int *) calloc(N, sizeof(int));

    for(int i = 0; i < N; i++)
    {
        houses[i] = HOUSE_MIN + rand() % (HOUSE_MAX - HOUSE_MIN + 1);
        pigs[i].house_pointer = houses + i;
        if (pthread_mutex_init(&pigs[i].mutex, NULL) != 0) {
            ERR("pthread_mutex_init()");
        }
    }

    //ignore SIGINT
    sethandler(SIG_IGN, SIGINT);

    print_array(houses, N);

    //create threads
    for(int i = 0; i < N; i++)
    {
        int err = pthread_create(&(pigs[i].tid), NULL, pig_work, (void *)&(pigs[i]));
        if(0 != err)
            ERR("Couldn't create thread");
    }

    //wolf
    struct timespec full = { 0, blow_interval * MILLITONANO};
    int slept_time = 0;
    int seed = getpid();
    while(slept_time <= T * 1000)
    {
        struct timespec st = full;
        while(nanosleep(&st, &st) != 0);
        slept_time += blow_interval;
        wolf_blow(houses, N, pigs, &seed);
    }

    //join threads
    for(int i = 0; i < N; i++)
    {
        int err = pthread_cancel(pigs[i].tid);
        if(0 != err)
            ERR("Couldn't cancel thread");
        err = pthread_join(pigs[i].tid, NULL);
        if(0 != err)
            ERR("Couldn't join thread");
    }

    print_array(houses, N);

    free(pigs);

    return EXIT_SUCCESS;
}

void *pig_work(void *pig_arg)
{
    pig_info *arg = (pig_info *)pig_arg;
    unsigned int seed = *arg->house_pointer;
    int delay = SLEEP_MIN + rand_r(&seed) % (SLEEP_MAX - SLEEP_MIN + 1);
    struct timespec ts = {0, delay * MILLITONANO};
    while (1)
    {
        nanosleep(&ts, NULL);
        pthread_mutex_lock(&arg->mutex);
        if(arg->house_pointer[0] == 0)
        {
            pthread_mutex_unlock(&arg->mutex);
            break;
        }
        arg->house_pointer[0]++;// = arg->house_pointer[0] + 1;
        pthread_mutex_unlock(&arg->mutex);
    }
    kill(getpid(), SIGINT);
    return NULL;
}

void print_array(int *arr, int size)
{
    printf("[ ");
    for(int i = 0; i < size; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("]\n");
}

int blow_strength(int *houses, int size, unsigned int *seed)
{
    int max = houses[0];
    for(int i = 1; i < size; i++)
    {
        if(houses[i] > max)
            max = houses[i];
    }
    int ret = BLOW_B_MIN + rand_r(seed) % (max - BLOW_B_MIN + 1);
    return ret;
}

void wolf_blow(int *houses, int size, pig_info *pigs, unsigned int *seed)
{
    int b = blow_strength(houses, size, seed);
    int house = rand_r(seed) % size;
    printf("Blowing on house %d with strength %d\n", house + 1, b);
    for(int i = 0; i <= house; i++)
    {
        if(houses[i] < b)
        {
            pthread_mutex_lock(&pigs->mutex);
            houses[i] = 0;
            pthread_mutex_unlock(&pigs->mutex);
        }
    }
    print_array(houses, size);
}

void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}
