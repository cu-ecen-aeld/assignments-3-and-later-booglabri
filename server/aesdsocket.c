#include "aesdsocket.h"

// Debug macros
#define DEBUG(msg, ...)
//#define DEBUG(msg, ...) fprintf(stderr, "DEBUG: " msg, ##__VA_ARGS__)

//===================================================================
// Singly link list entry structure
//===================================================================
struct entry {
    struct thread_data data;
    SLIST_ENTRY(entry) entries;
};    
SLIST_HEAD(slisthead, entry);

//===================================================================
// signal handler
//===================================================================

bool sigcaught = false;

void signal_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) sigcaught = true;
}

//===================================================================
// main
//===================================================================

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;
    struct sockaddr_storage client_addr;
    struct sockaddr_in *client_addrptr = (struct sockaddr_in *)&client_addr;
    socklen_t client_addrsize;
    int sockfd, clientfd;
    int status, tcnt = -1;
    int yes=1;
    char client_ipstr[INET_ADDRSTRLEN];
    struct sigaction act;
    struct slisthead head;
    struct entry *node;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t tsthread;
    struct pollfd ufds[1];
    time_t t;
    struct tm *ts;
    struct ts_data timestamp;

    // Assign mutex to timestamp data structure
    timestamp.mutex = &mutex;
    
    // Initialize the thread queue
    SLIST_INIT(&head);
    
    // Setup up signal handling
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = signal_handler;
    if (sigaction(SIGINT, &act, NULL) == -1) {
	perror("sigaction SIGINT");
	exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &act, NULL) == -1) {
	perror("sigaction SIGTERM");
	exit(EXIT_FAILURE);
    }

    // Open syslog
    openlog(argv[0], LOG_CONS | LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);

    // Setup getaddrinfo hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get address info
    if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
	fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	exit(EXIT_FAILURE);
    }

    // Check that only one address exists
    //DEBUG("res->ai_next: %p\n", (void *)res->ai_next);
    //for (int *p = res; p != NULL; p = p->ai_next) {
    //	if (p->ai_family == AF_INET) break;
    //}
    if (res->ai_next != NULL) {
	fprintf(stderr, "More that one address identified\n");
	exit(EXIT_FAILURE);
    }

    // Create socket
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
	perror("socket");
	exit(EXIT_FAILURE);
    }

    // Configure socket to be non-blocking
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
	perror("fcntl");
    	exit(EXIT_FAILURE);
    }
    
    // Set socket options to reuse address
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
	perror("setsockopt");
	exit(EXIT_FAILURE);
    }

    // Bind socket to port
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
	perror("bind");
	exit(EXIT_FAILURE);
    }

    // Switch to daemon mode if argument -d is passed
    DEBUG("argc: %d argv[1]: %s\n", argc, argv[1]);
    if (argc == 2 && strncmp(argv[1], "-d", 2) == 0) {
	DEBUG("daemon mode\n");
	if (daemon(0, 0) == -1) {
	    perror("daemon");
	    exit(EXIT_FAILURE);
	}
    }

    // Listen for client connection request
    if (listen(sockfd, BACKLOG) == -1) {
	perror("listen");
	exit(EXIT_FAILURE);
    }

    // Set up pollfd structure
    ufds[0].fd = sockfd;
    ufds[0].events = POLLIN | POLLPRI;
    
    // Loop listening for clients until SIGINT or SIGTERM is received
    while(true) {

	// Poll sock file descripters for read ready
        if ((status = poll(ufds, 1, 1000)) == -1) {
	    if (sigcaught) goto CLEANUP;
	    perror("poll");
	    exit(EXIT_FAILURE);
	} else if (status == 0) {
	    tcnt = (tcnt + 1) % DELAYSECS;

	    // Get timestamp
	    if (tcnt == 0) {
		t = time(NULL);
		if ((ts = localtime(&t)) == NULL) {
		    perror("localtime");
		    exit(EXIT_FAILURE);
		}
		if (strftime(timestamp.timestr, sizeof(timestamp.timestr), "timestamp: %a, %d %b %Y %T %z", ts) == 0) {
		    fprintf(stderr, "strftime returned 0");
		    exit(EXIT_FAILURE);
		}
		DEBUG("%s\n", timestamp.timestr);
	    
		// Create and spawn timestamp thread
		if ((status = pthread_create(&tsthread, NULL, timethread, (void *)&timestamp)) != 0) {
		    fprintf(stderr, "pthread_create: %s\n", strerror(status));
		    exit(EXIT_FAILURE);
		}

		// Join threads that have exited
		if ((status = pthread_join(tsthread, NULL)) != 0) {
		    fprintf(stderr, "pthread_join: %s\n", strerror(status));
		    exit(EXIT_FAILURE);
		}
	    }
	}
	DEBUG("Poll result: %d  tcnt: %d\n", status, tcnt);

        // Accept connection from client
	client_addrsize = sizeof client_addr;
        if ((clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrsize)) == -1) {
	    if (errno != EAGAIN && errno != EWOULDBLOCK) {
		perror("accept");
		exit(EXIT_FAILURE);
	    }
	} else {
	    // Beginning of client thread spawn

	    // Log accepted client IP to syslog
	    if (inet_ntop(client_addrptr->sin_family, (void *)&client_addrptr->sin_addr, client_ipstr, sizeof client_ipstr) == NULL) {
		perror("inet_ntop");
		exit(EXIT_FAILURE);
	    }
	    syslog(LOG_INFO, "Accepted connection from %s\n", client_ipstr);

	    // Allocate a thread node entry and insert at head of thread queue
	    if ((node = malloc(sizeof(struct entry))) == NULL) {
		perror("malloc node");
		exit(EXIT_FAILURE);
	    }
	    node->data.clientfd = clientfd;
	    strncpy(node->data.client_ipstr, client_ipstr, INET_ADDRSTRLEN);
	    node->data.tid = (pthread_t)NULL;
	    node->data.mutex = &mutex;
	    node->data.thread_info_ptr = &node->data;
	    node->data.thread_complete_success = false;
	    SLIST_INSERT_HEAD(&head, node, entries);
	
	    // Create and spawn client thread
	    if ((status = pthread_create(&node->data.thread, NULL, clientthread, (void *)&node->data)) != 0) {
		fprintf(stderr, "pthread_create: %s\n", strerror(status));
		exit(EXIT_FAILURE);
	    }
	} // End of client thread spawn

	// Loop over thread queue checking for completed clients
	SLIST_FOREACH(node, &head, entries) {
	    DEBUG("thread running: %lu %lu (%d)\n", node->data.thread, node->data.tid, node->data.thread_complete_success);
	    if (node->data.thread_complete_success) {
		DEBUG("thread completed: %lu\n", node->data.thread);
		
		// Join threads that have exited
		if ((status = pthread_join(node->data.thread, NULL)) != 0) {
			fprintf(stderr, "pthread_join: %s\n", strerror(status));
			exit(EXIT_FAILURE);
		}
	
		// Close client file descriptor
		if (close(node->data.clientfd) == -1) {
			perror("client close");
			exit(EXIT_FAILURE);
		}
    	
		// Log closed client IP address to syslog
		syslog(LOG_INFO, "Closed connection from %s\n", node->data.client_ipstr);

		// Remove thread from thread queue and free node heap memory
		SLIST_REMOVE(&head, node, entry, entries);

		break;
	    }
	} // End of SLIST loop

	// Free complete client node from the thread queue
	free(node);

    } // End of client loop
    
 CLEANUP:

    // Handle exceptions SIGINT and SIGTERM
    if (sigcaught) {
	syslog(LOG_INFO, "Caught signal, exiting");
	if (access(DATAFILE, F_OK) == 0) {
	    if (remove(DATAFILE) == -1) {
		perror("remove");
		exit(EXIT_FAILURE);
	    }
	}
    }

    // Remove thread from thread queue and free node heap memory
    while (!SLIST_EMPTY(&head)) {
	node = SLIST_FIRST(&head);
	SLIST_REMOVE_HEAD(&head, entries);
	free(node);
    }

    // Shutdown network connection
    if (shutdown(sockfd, SHUT_RDWR) == -1) {
	perror("shutdown");
	exit(EXIT_FAILURE);
    }

    // Close socket file descriptor
    if (close(sockfd) == -1) {
	perror("socket close");
	exit(EXIT_FAILURE);
    }

    // Free address info
    freeaddrinfo(res);

    // Close syslog
    closelog();

    // Exit cleanly
    exit(EXIT_SUCCESS);
}
