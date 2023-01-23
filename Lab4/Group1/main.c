#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#ifndef _POSIX_ASYNCHRONOUS_IO
#error System does not support asyncrhonous I/O
#endif

#define ERR(source) (perror(source),\
		     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
		     exit(EXIT_FAILURE))


void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s filename block_size_in_bytes \n", pname);
    exit(EXIT_FAILURE);
}

volatile sig_atomic_t work = 0;
off_t get_file_length(int fd);
void print_buf(char *buf, int n);
void thread_work(sigval_t sigval);

int main(int argc, char **argv)
{
    if(argc != 3)
        usage(argv[0]);
    int n = atoi(argv[2]);
    if(n <= 0)
        usage(argv[0]);
    int fildes = open(argv[1], O_RDWR);
    if(fildes < 0)
    {
        ERR("open");
    }
    off_t length = get_file_length(fildes);
    int blocks = length / n;
    /*part 1.
    char *buffer = (char *) calloc(n, sizeof(char));
    if(NULL == buffer)
        ERR("calloc");
    size_t bytes_read = read(fildes, buffer, n);
    print_buf(buffer, bytes_read);
    free(buffer);
    */

//    char *buffer = (char *) calloc(n, sizeof(char));
//    if(NULL == buffer)
//        ERR("calloc");

    struct aiocb cb[blocks];
    struct aiocb * cb_pointers[blocks];
    for(int i = 0; i < blocks; i++)
    {
        memset(&cb[i], 0, sizeof(struct aiocb));
        cb[i].aio_fildes = fildes;
        cb[i].aio_buf = (char *) calloc(n, sizeof(char));
        if(NULL == cb[i].aio_buf)
            ERR("calloc");
        cb[i].aio_offset = i * n;
        cb[i].aio_nbytes = n;

        //thread setup
        cb[i].aio_sigevent.sigev_notify = SIGEV_THREAD;
        cb[i].aio_sigevent.sigev_notify_function = thread_work;
        cb[i].aio_sigevent.sigev_notify_attributes = NULL;
        cb[i].aio_sigevent.sigev_value.sival_ptr = &cb[i];

        cb_pointers[i] = &cb[i];
    }

    cb[blocks - 1].aio_nbytes = length % n;

    alarm(3);

    for(int i = 0; i < blocks; i++)
    {
        fprintf(stderr, "offset: %d\n", cb[i].aio_offset);
        if(aio_read(cb_pointers[i]))
            ERR("aio_read");
        if(aio_suspend((const struct aiocb *const *)(cb_pointers + i), 1, NULL) < 0)
            ERR("aio_suspend");
    }

    while(1);
    /*
     for (int j = 0; j < blocks ++j) {
            if (cb_pointers[j] == NULL) {
                printf("read[%d] done\n", j);
                continue;
            }

            if (aio_error(cb_pointers[j]) == EINPROGRESS) {
                printf("read[%d] is still in progress\n", j);
                continue;
            } else {

            }
        }
     */

    //free(buffer);
    close(fildes);
    return EXIT_SUCCESS;
}

off_t get_file_length(int fd)
{
    struct stat buf;
    if (fstat(fd, &buf) == -1)
        ERR("Cannot fstat file");
    return buf.st_size;
}

void print_buf(char *buf, int n)
{
    for(int i = 0; i < n; i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");
}

void thread_work(sigval_t sigval)
{
    struct aiocb *cb = (struct aiocb *)sigval.sival_ptr;
    int ret = aio_error(cb);
    printf("aio_error() returned %d\n", ret);

    ssize_t bytes_read = aio_return(cb);
    printf("aio_read returned %ld\n", bytes_read);
    char c;
    char *buffer = (char *)cb->aio_buf;
    for(int i = 0; i < bytes_read; i++)
    {
        c = buffer[i];
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        {
            //do nothing - I know it's bad code
        }
        else
        {
            buffer[i] = ' ';
        }
    }
    print_buf(buffer, bytes_read);
}
