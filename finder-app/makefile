CC=$(CROSS_COMPILE)gcc
CFLAGS=-I.
DEPS=
OBJ=writer.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

writer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

all: writer

.PHONY: clean

clean: 
	rm -f $(OBJ) writer
