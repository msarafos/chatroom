#ifndef __SERVERLIB_H_
#define __SERVERLIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int

#define PORT "8080"
#define BACKLOG 10

/* network functions. */
extern void   config_local_addr(const char *host, const char *port, struct addrinfo *hints, struct addrinfo **res);
extern SOCKET socket_creation(int domain, int socket_type, int protocol, struct addrinfo *hints);
extern void   server_options(int sockfd, int level, int optname, void *optval, socklen_t optlen);
extern void   bind_to_addr(int sockfd, struct addrinfo *bind_address);
extern void   listening(int sockfd, int backlog);

/* clients' struct. */
typedef struct client_node
{
    struct client_node *previous;
    struct client_node *next;
    char nickname[32];
    char IP[32];

} CLIENT_NODE;

/* client-list functions. */
extern void init_list(CLIENT_NODE **root);
extern int  find_client(CLIENT_NODE *root, char *nickname);
extern int  append_client(CLIENT_NODE **root, char *IP, char *nickname);
extern int  remove_client(CLIENT_NODE **root, char *nickname);
extern void print_list(CLIENT_NODE *root);

#endif
