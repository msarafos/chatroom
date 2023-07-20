all: tcp_serve_chat tcp_client
tcp_serve_chat: tcp_serve_chat.o serverlib.o
	gcc -Wall -g tcp_serve_chat.o serverlib.o -o tcp_serve_chat
tcp_client: tcp_client.o serverlib.o
	gcc -Wall -g tcp_client.o serverlib.o -o tcp_client

tcp_serve_chat.o: tcp_serve_chat.c serverlib.h
	gcc -Wall -g -c tcp_serve_chat.c
tcp_client.o: tcp_client.c serverlib.h
	gcc -Wall -g -c tcp_client.c

serverlib.o: serverlib.c serverlib.h
	gcc -Wall -g -c serverlib.c

clean: 
	rm -f *.o tcp_serve_chat tcp_client
