CC=$(CROSS_COMPILE)gcc
CFLAGS=-I.
DEPS=
OBJ=writer.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

writer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean: 
	rm -f $(OBJ) writer
