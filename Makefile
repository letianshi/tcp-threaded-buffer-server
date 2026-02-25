CFLAGS = -Wall -Werror -g

all: bufferServer bufferServerFaulty client

bufferServer: bufferServer.o
	$(CC) $(CFLAGS) -o bufferServer bufferServer.o -lpthread

bufferServer.o: bufferServer.c
	$(CC) $(CFLAGS) -c bufferServer.c

bufferServerFaulty: bufferServerFaulty.o
	$(CC) $(CFLAGS) -o bufferServerFaulty bufferServerFaulty.o -lpthread

bufferServerFaulty.o: bufferServerFaulty.c
	$(CC) $(CFLAGS) -c bufferServerFaulty.c


client: client.o
	$(CC) $(CFLAGS)  -o client client.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o bufferServer bufferServerFaulty client
