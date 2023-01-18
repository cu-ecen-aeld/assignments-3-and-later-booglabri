#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <syslog.h>

//#define DEBUG(msg, ...)
#define DEBUG(msg, ...) fprintf(stderr, "DEBUG: " msg, ##__VA_ARGS__)

#define PORT "9000"
#define BACKLOG 5
#define DATAFILE "/var/tmp/aesdsocketdata"
#define BUFSIZE 5

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;
    struct sockaddr_storage client_addr;
    struct sockaddr_in *client_addrptr = (struct sockaddr_in *)&client_addr;
    socklen_t client_addrsize;
    int sockfd, clientfd, fd;
    int flags;
    int status;
    int yes=1;
    char client_ipstr[INET_ADDRSTRLEN];
    char *buf, *buf2, *bufa, *bufb, *startline, *endline, *rbuf;
    bool bflag;
    ssize_t len, cnt, cnt2;

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
    //DEBUG("res->ai_next: %p\n", (void *)res->ai_next);
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

    // Loop listening for clients unti SIGINT or SIGTERM is received
    while (1) {
	
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

        // Open data file, create if does not exist
        flags = O_RDWR | O_APPEND;
        if (access(DATAFILE, F_OK) != 0) flags |= O_CREAT;
        if ((fd = open(DATAFILE, flags, 0644)) == -1) {
	    perror("open");
	    exit(-1);
        }
    	
        // Read incoming socket data stream, write to data file and return response to outgoing data stream
        bufa = (char *)malloc(BUFSIZE+1);
        bufb = (char *)malloc(BUFSIZE+1);
        rbuf = (char *)malloc(BUFSIZE+1);
        bflag = true;
        buf2 = bufb;
        buf2[0] = '\0';
        do {
	    if (bflag) {
		buf = bufa;
		bflag = false;
	    } else {
		buf = bufb;
		bflag = true;
	    }
	    if ((len = recv(clientfd, (void *)buf, BUFSIZE, 0)) == -1) {
		perror("recv");
		exit(-1);
	    }
	    buf[len] = '\0';
	    DEBUG("len: %d\n", (int)len);
	    DEBUG("buf: |%s|\n", buf);
	    startline = buf;
	    if (len > 0) {
		while ((endline = index(startline, '\n')) != 0) {
		    cnt = endline - startline + 1;
		    cnt2 = index(buf2, '\0') - buf2;
		    DEBUG("[%ld] startline: %p -> 0x%02x (%c) endline: %p -> 0x%02x\n",
			  cnt, startline, *startline, *startline, endline, *endline);
		    if ((write(fd, buf2, cnt2)) != cnt2) {
			perror("write");
			exit(-1);
		    }
		    if ((write(fd, startline, cnt)) != cnt) {
			perror("write");
			exit(-1);
		    }
		    startline = endline + 1;
		}
		buf2 = startline;
		if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		    perror("lseek");
		    exit(-1);
		}
		while ((cnt = read(fd, rbuf, BUFSIZE)) != 0) {
		    if ((send(clientfd, (void *)rbuf, cnt, 0)) == -1) {
			perror("send");
			exit(-1);
		    }
		}
		//char tbuf[2] = "\n\0";
		//if ((send(clientfd, (void *)tbuf, 2, 0)) == -1) {
		//    perror("send");
		//    exit(-1);
		//}
	    }
        } while (len > 0);
        free(bufa);
        free(bufb);
        free(rbuf);

        // Close data file
        close(fd);
        
        // Close client file descriptor
        if (close(clientfd) == -1) {
	    perror("client close");
	    exit(-1);
        }
    	
        // Log closed client IP address to syslog
        syslog(LOG_INFO, "Closed connection from %s\n", client_ipstr);

    } // End of client loop
    
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
