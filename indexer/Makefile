#CFLAGS= -Wall -pedantic -std=c11 -I$(HOME)/engs50/tse/utils -L$(HOME)/engs50/tse/lib -g
#
#LIBS=-lutils -lcurl
#
#indexer: indexer.o pageio.o
#	gcc $(CFLAGS) $^ $(LIBS) -o $@
#
#%.o: %.c %.h
#	gcc $(CFLAGS) -c $< -o $@
#
#clean:
#	rm -f indexer *.o


CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -I$(HOME)/engs50/tse/utils -g
LDFLAGS = -L$(HOME)/engs50/tse/lib
LDLIBS = -lutils -lcurl

indexer: indexer.o
	$(CC) $(CFLAGS) indexer.o $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f indexer *.o
