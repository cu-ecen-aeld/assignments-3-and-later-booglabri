#include "aesdsocket.h"

// Debug macros
#define DEBUG(msg, ...)
//#define DEBUG(msg, ...) fprintf(stderr, "DEBUG: " msg, ##__VA_ARGS__)

//===================================================================
// client thread
//===================================================================

void *timethread(void *thread_param)
{
    int fd, status;
    struct ts_data *thread_args = (struct ts_data *)thread_param;

    // Acquire mutex lock to guard simultaneous file I/O
    if ((status = pthread_mutex_lock(thread_args->mutex)) != 0) {
	fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(status));
	goto EXIT;
    }

    // Set thread ID
    thread_args->tid = pthread_self();

    // Open data file, create if does not exist
    if (access(DATAFILE, F_OK) == 0) {
	if ((fd = open(DATAFILE, O_WRONLY | O_APPEND, 0644)) == -1) {
	    perror("open");
	    goto EXIT;
	}
    } else {
	goto EXIT;
    }

    // write timestamp to file
    if ((write(fd, thread_args->timestr, sizeof(thread_args->timestr))) != sizeof(thread_args->timestr)) {
	perror("write");
	goto EXIT;
    }

    // write newline to file
    if ((write(fd, "\n", 1)) != 1) {
	perror("write");
	goto EXIT;
    }

 EXIT:
    // Close data file
    close(fd);

    // Release mutex lock
    if ((status = pthread_mutex_unlock(thread_args->mutex)) != 0) {
	fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(status));
	goto EXIT;
    }

    // Exit thread
    return thread_param;
}
