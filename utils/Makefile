CC = gcc
CFLAGS=-Wall -pedantic -std=c11 -I. -g
OFILES=queue.o hash.o webpage.o pageio.o


all: $(OFILES)
	ar cr ~/engs50/tse/lib/libutils.a $(OFILES)

hash: queue.o hash.o 
	$(CC) $(CFLAGS) $^ -o $@

queue:  queue.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o  queue hash ~/engs50/tse/lib/libutils.a
