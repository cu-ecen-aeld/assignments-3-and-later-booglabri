#ifndef AESDSOCKET_H
#define AESDSOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <netdb.h>
#include <syslog.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

// Parameters
#define PORT "9000"
#define BACKLOG 5
#define DATAFILE "/var/tmp/aesdsocketdata"
#define BUFSIZE 1024
#define TSBUFSIZE 50
#define DELAYSECS 10

// Timestamp thread data structure
struct ts_data {
    char timestr[TSBUFSIZE];
    pthread_mutex_t *mutex;
    pthread_t tid;
};

// Thread data structure
struct thread_data {
    int clientfd;
    char client_ipstr[INET_ADDRSTRLEN];
    pthread_mutex_t *mutex;
    pthread_t thread;
    pthread_t tid;
    struct thread_data *thread_info_ptr;
    bool thread_complete_success;
};

// Client thread
void *clientthread(void *thread_param);

// Timestamp thread
void *timethread(void *timestamp);

#endif
