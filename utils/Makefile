CC = gcc
CFLAGS=-Wall -pedantic -std=c11 -I. -g
OFILES= lhash.o lqueue.o queue.o hash.o webpage.o pageio.o indexio.o


all: $(OFILES)
	ar cr ~/engs50/tse/lib/libutils.a $(OFILES)

hash: queue.o hash.o 
	$(CC) $(CFLAGS) $^ -o $@

queue:  queue.o
	$(CC) $(CFLAGS) $^ -o $@

test_lhash: test_lhash.c lhash.o hash.o queue.o lqueue.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o test_lhash lhash lqueue queue hash  webpage.o pageio.o indexio.o ~/engs50/tse/lib/libutils.a
