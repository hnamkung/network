all : server client

server : server.o io_library.o
	gcc -o server server.o io_library.o

client : client.o io_library.o 
	gcc -o client client.o io_library.o

server.o : server.c 
	gcc -c server.c 

client.o : client.c 
	gcc -c client.c 

io_library.o : io_library.h io_library.c
	gcc -c io_library.c

clean :
	rm -rf server server.o client client.o io_library.o

