CFLAGS= -Wall -pedantic -std=c11 -I$(HOME)/engs50/tse/utils -L$(HOME)/engs50/tse/lib -g

LIBS=-lutils -lcurl

indexer: indexer.o pageio.o
	gcc $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c %.h
	$(CFLAGS) -c $< -o $@

clean:
	rm -f indexer *.o
