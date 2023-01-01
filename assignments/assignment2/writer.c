#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char *argv[])
{
  int fd;
  size_t count;
  ssize_t nr;

  openlog(argv[0], LOG_CONS | LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);
  
  if (argc != 3) {
    syslog(LOG_ERR, "Error: invalid number of arguments %d, expected 2\n", argc-1);
    syslog(LOG_ERR, "Usage: %s file_to_write string_write_into_file\n", argv[0]);
    exit(1);
  }

  fd = creat(argv[1], 0644);
  if (fd == -1) {
    syslog(LOG_ERR, "Error: opening file %s\n", argv[1]);
    exit(1);
  }

  count = strlen(argv[2]);
  nr = write(fd, argv[2], count);
  if (nr == -1) {
    syslog(LOG_ERR, "Error: writing file %s\n", argv[1]);
    exit(1);
  } else if (nr != count) {
    syslog(LOG_ERR, "Error: invalid bytes written to file %ld, expected %ld\n", nr, count);
    exit(1);
  } else {
    syslog(LOG_DEBUG, "Writing %s to %s\n", argv[2], argv[1]);
  }

  closelog();
  exit(0);
  }
