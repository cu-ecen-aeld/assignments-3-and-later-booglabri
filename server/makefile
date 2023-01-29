ifeq ($(CC),)
	CC=$(CROSS_COMPILE)gcc
endif
ifeq ($(CFLAGS),)
	CFLAGS=-I. -Wall -Werror -g -O0
endif
ifeq ($(LDFLAGS),)
	LDFLAGS=-pthread
endif

DEPS=aesdsocket.h
OBJS=aesdsocket.o clientthread.o timethread.o
TARGET=aesdsocket

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

showip: showip.o
	$(CC) -o $@ $^ $(LDFLAGS)

all: $(TARGET)

.PHONY: clean cleanall

clean:
	rm -f $(OBJS) $(TARGET)

cleanall: clean
	rm -f showip.o showip
