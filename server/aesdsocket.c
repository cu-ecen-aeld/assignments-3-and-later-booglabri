#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

//#define DEBUG(msg, ...)
#define DEBUG(msg, ...) fprintf(stderr, "DEBUG: " msg "\n", ##__VA_ARGS__)

#define PORT "9000"
#define BACKLOG 5

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;
    struct sockaddr_storage client_addr;
    struct sockaddr_in *client_addrptr = (struct sockaddr_in *)&client_addr;
    socklen_t client_addrsize;
    int sockfd, clientfd;
    int status;
    int yes=1;
    char client_ipstr[INET_ADDRSTRLEN];

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
	exit(-1);
    }

    // Check that only one address exists
    //DEBUG("res->ai_next: %p", (void *)res->ai_next);
    //for (int *p = res; p != NULL; p = p->ai_next) {
    //	if (p->ai_family == AF_INET) break;
    //}
    if (res->ai_next != NULL) {
	fprintf(stderr, "More that one address identified\n");
	exit(-1);
    }

    // Create socket
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
	perror("socket");
	exit(-1);
    }

    // Set socket options to reuse address
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
	perror("setsockopt");
	exit(-1);
    }

    // Bind socket to port
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
	perror("bind");
	exit(-1);
    }

    // Listen for client connection request
    if (listen(sockfd, BACKLOG) == -1) {
	perror("listen");
	exit(-1);
    }

    // Accept connection from client
    client_addrsize = sizeof client_addr;
    if ((clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrsize)) == -1) {
	perror("accept");
	exit(-1);
    }
    if (inet_ntop(client_addrptr->sin_family, (void *)&client_addrptr->sin_addr, client_ipstr, sizeof client_ipstr) == NULL) {
	perror("inet_ntop");
	exit(-1);
    }

    // Log accepted client IP to syslog
    syslog(LOG_INFO, "Accepted connection from %s\n", client_ipstr);

    // do work


    // Log closed client IP address to syslog
    syslog(LOG_INFO, "Closed connection from %s\n", client_ipstr);
    
    // Close client file descriptor
    if (close(clientfd) == -1) {
	perror("client close");
	exit(-1);
    }
	
    // Shutdown network connection
    if (shutdown(sockfd, SHUT_RDWR) == -1) {
	perror("shutdown");
	exit(-1);
    }

    // Close socket file descriptor
    if (close(sockfd) == -1) {
	perror("socket close");
	exit(-1);
    }

    // Free address info
    freeaddrinfo(res);

    // Close syslog
    closelog();

    // Exit cleanly
    exit(0);
}
