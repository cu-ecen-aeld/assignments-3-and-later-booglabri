#include "aesdsocket.h"

// Debug macros
#define DEBUG(msg, ...)
//#define DEBUG(msg, ...) fprintf(stderr, "DEBUG: " msg, ##__VA_ARGS__)

//===================================================================
// client thread
//===================================================================

void *clientthread(void *thread_param)
{
    int fd, flags, status;
    char *buf = NULL, *rbuf = NULL;
    ssize_t len, cnt;
    struct thread_data *thread_args = (struct thread_data *)thread_param;

    // Acquire mutex lock to guard simultaneous file I/O
    if ((status = pthread_mutex_lock(thread_args->mutex)) != 0) {
	fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(status));
	goto EXIT;
    }

    // Set thread ID
    thread_args->tid = pthread_self();

    // Open data file, create if does not exist
    flags = O_RDWR | O_APPEND;
    if (access(DATAFILE, F_OK) != 0) flags |= O_CREAT;
    if ((fd = open(DATAFILE, flags, 0644)) == -1) {
	    perror("open");
	    goto EXIT;
    }

    // Allocate buffers
    if ((buf = (char *)malloc(BUFSIZE+1)) == NULL) {
	perror("malloc buf\n");
	goto EXIT;
    }
    if ((rbuf = (char *)malloc(BUFSIZE+1)) == NULL) {
	perror("malloc rbuf");
	goto EXIT;
    }

    // Read incoming socket data stream, write to data file and return response to outgoing data stream
    do {
	// read incoming socket data stream
	if ((len = recv(thread_args->clientfd, (void *)buf, BUFSIZE, 0)) == -1) {
	    perror("recv");
	    goto EXIT;
	}
	
	// write to data file
	buf[len] = '\0';
	DEBUG("recv len: %d  buf: |%s|\n", (int)len, buf);
	if ((write(fd, buf, len)) != len) {
	    perror("write");
	    goto EXIT;
	}

	// data packet found
	if (index(buf, '\n') != 0) {

	    // seek to beginning of file
	    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		perror("lseek");
		goto EXIT;
	    }
		
	    // return file contents in outgoing data stream
	    while ((cnt = read(fd, rbuf, BUFSIZE)) != 0) {
		rbuf[cnt] = '\0';
		DEBUG("send cnt: %d rbuf: |%s|\n", (int)cnt, rbuf);
		if ((cnt = send(thread_args->clientfd, (void *)rbuf, cnt, 0)) == -1) {
		    perror("send");
		    goto EXIT;
		}
		DEBUG("sent cnt: %d\n", (int)cnt);
	    }
	}
	
    } while (len > 0); // socket data stream closed

    // Set thread complete success
    thread_args->thread_complete_success = true;

 EXIT:
    // Free buffers
    if (!NULL) free(buf);
    if (!NULL) free(rbuf);

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
