ifeq ($(CC),)
	CC=$(CROSS_COMPILE)gcc
endif
ifeq ($(CFLAGS),)
	CFLAGS=-I. -Wall -Werror -pthread
endif
ifeq ($(LDFLAGS),)
	LDFLAGS= -pthread
endif

%.o: %.c %.h
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
