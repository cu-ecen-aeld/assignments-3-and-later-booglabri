#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n", ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

// Milliseconds sleep macro
#define msleep(tms) usleep(tms * 1000);

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    msleep(thread_func_args->wait_to_obtain_ms);
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    DEBUG_LOG("pthread_mutex_lock return %d", rc);
    if (rc != 0) {
        ERROR_LOG("pthread_mutex_lock failed with %d", rc);
    } else {
	DEBUG_LOG("tid before %ld", thread_func_args->tid);
	thread_func_args->tid = pthread_self();
	DEBUG_LOG("tid after %ld", thread_func_args->tid);
	DEBUG_LOG("thread %ld", *thread_func_args->thread);
	msleep(thread_func_args->wait_to_release_ms);
	thread_func_args->thread_complete_success = true;
	rc = pthread_mutex_unlock(thread_func_args->mutex);
	if (rc != 0) {
	    ERROR_LOG("pthread_mutex_unlock failed with %d", rc);   
	}
    }
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data* thread_info = (struct thread_data *)malloc(sizeof(struct thread_data));
    if (thread_info == NULL) {
	return false;
    }

    thread_info->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_info->wait_to_release_ms = wait_to_release_ms;
    thread_info->thread = thread;
    thread_info->mutex = mutex;
    thread_info->tid = (pthread_t)NULL;
    thread_info->thread_info_ptr = thread_info;
    thread_info->thread_complete_success = false;
    
    int ret = pthread_create(thread, NULL, threadfunc, (void *)thread_info);
    DEBUG_LOG("pthread_create return %d", ret);
    DEBUG_LOG("thread %p", thread);
    DEBUG_LOG("thread_info->thread %p", thread_info->thread);
    if (ret != 0) {
	ERROR_LOG("pthread_create return %d", ret);
	return false;
    }

    return true;
}

