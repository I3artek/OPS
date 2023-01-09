#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t sig_count = 0;

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s m b s name\n", name);
    fprintf(stderr, "m - number of 1/1000 milliseconds between signals [1,999], "
                    "i.e. one milisecond maximum\n");
    fprintf(stderr, "b - number of blocks > 0\n");
    fprintf(stderr, "s - size of one block\n");
    fprintf(stderr, "name - name of the file to be created\n");
    exit(EXIT_FAILURE);
}

void sethandler(void (*f)(int), int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = f;
    if(-1 == sigaction(sig, &sa, NULL))
        ERR("sigaction");
}

void sigusr1_handler(int sig)
{
        sig_count++;
}

void child_work(int m)
{
    struct timespec t = { 0, m * 1000};
    sethandler(SIG_DFL, SIGUSR1);
    while(1)
    {
        nanosleep(&t, NULL);
        if(kill(getppid(), SIGUSR1))
            ERR("kill");
    }
}

ssize_t bulk_read_not(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do {
        //???
        while((c = read(fd, buf, count)) < 0 && EINTR == errno);
        if(c < 0)
            return c;
        if(0 == c)
            return len;//EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_write_not(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do {
        while((c = write(fd, buf, count)) < 0 && EINTR == errno);
        if(c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len; // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}


ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

void parent_work_old(int b, int s, char *name)
{
    int i, in, out;
    ssize_t count;
    char *buf = malloc(s);
    if(!buf)
        ERR("malloc");
    if((out = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0)
        ERR("open");
    if((in = open("/dev/urandom", O_RDONLY)) < 0)
        ERR("open");
    for(i = 0; i < b; i++)
    {
        if((count = read(in, buf, s)) < 0)
            ERR("read");
        if((count = write(out, buf, count)) < 0)
            ERR("write");
        if(fprintf(stderr, "Block of %ld bytes transfered. Signals RX:%d\n", count, sig_count))
            ERR("fprintf");
    }
    if(close(in))
        ERR("close");
    if(close(out))
        ERR("close");
    free(buf);
    if(kill(0, SIGUSR1))
        ERR("kill");
}

void parent_work(int b, int s, char name)
{
    int i, in, out;
    ssize_t count;
    char buf = malloc(s);
    if (!buf)
        ERR("malloc");
    if ((out = TEMP_FAILURE_RETRY(open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777))) < 0)
        ERR("open");
    if ((in = TEMP_FAILURE_RETRY(open("/dev/urandom", O_RDONLY))) < 0)
        ERR("open");
    for (i = 0; i < b; i++) {
        if ((count = bulk_read(in, buf, s)) < 0)
            ERR("read");
        if ((count = bulk_write(out, buf, count)) < 0)
            ERR("read");
        if (TEMP_FAILURE_RETRY(
                fprintf(stderr, "Block of %ld bytes transfered. Signals RX:%d\n", count, sig_count)) < 0)
            ERR("fprintf");
    }
    if (TEMP_FAILURE_RETRY(close(in)))
        ERR("close");
    if (TEMP_FAILURE_RETRY(close(out)))
        ERR("close");
    free(buf);
    if (kill(0, SIGUSR1))
        ERR("kill");
}

int main(int argc, char **argv)
{
    if(argc < 5)
        usage(argv[0]);

    int m = atoi(argv[1]);
    int b = atoi(argv[2]);
    int s = atoi(argv[3]);
    char *name = argv[4];

    if (m <= 0 || m > 999 || b <= 0 || b > 999 || s <= 0 || s > 999)
        usage(argv[0]);
    sethandler(sigusr1_handler, SIGUSR1);
    pid_t pid;
    if((pid = fork()) < 0)
        ERR("fork");
    if(0 == pid)
        child_work(m);
    else
    {
        parent_work(b, s * 1024 * 1024, name);
        while(wait(NULL) > 0);
    }

    return EXIT_SUCCESS;
}
