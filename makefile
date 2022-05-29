CC=g++

all: client server
	
client: client.cpp
	$(CC) $^ -o client

server: server.cpp stack.cpp
	$(CC) $^ -o server -lpthread

clean:
	rm -f *.o server client