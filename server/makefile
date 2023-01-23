CC=$(CROSS_COMPILE)gcc
CFLAGS=-I. -Wall -Werror -pthread
LDFLAGS= -pthread
DEPS=

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

aesdsocket: aesdsocket.o
	$(CC) -o $@ $^ $(LDFLAGS)

showip: showip.o
	$(CC) -o $@ $^ $(LDFLAGS)

all: aesdsocket

.PHONY: clean cleanall

clean:
	rm -f aesdsocket.o aesdsocket

cleanall: clean
	rm -f showip.o showip
