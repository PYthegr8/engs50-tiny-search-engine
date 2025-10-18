CFLAGS= -Wall -pedantic -std=c11 -I$(HOME)/engs50/tse/utils -L$(HOME)/engs50/tse/lib -g

LIBS=-lutils -lcurl

crawler:
	gcc $(CFLAGS) crawler.c $(LIBS) -o $@

clean:
	rm -f crawler *.o
