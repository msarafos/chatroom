/* tcp_client.c */

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

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: ./tcp_client <hostname> <port>\n");
        exit(EXIT_FAILURE);
    }

    /* configuring server address. */
    printf("[+] Configuring remote address...\n");
    struct addrinfo hints, *client_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], argv[2], &hints, &client_address))
    {
        fprintf(stderr, "tcp client: getaddrinfo() failed.\n");
        exit(EXIT_FAILURE);
    }
    
    /* This is a good debugging technique. :) */
    printf("[+] Remote address (server) is: \n");
    char address_buffer[100] = {'\0'};
    char service_buffer[100] = {'\0'};
    
    getnameinfo(client_address->ai_addr, client_address->ai_addrlen, 
    address_buffer, sizeof(address_buffer), 
    service_buffer, sizeof(service_buffer), 
    NI_NUMERICHOST);
    printf("    Address buffer: %s | Service buffer: %s\n", address_buffer, service_buffer);


    /* socket creation. */
    printf("[+] Creating socket...\n");
    SOCKET socket_client = socket(client_address->ai_family, client_address->ai_socktype, client_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_client))
    {
        fprintf(stderr, "tcp client: socket() failed.\n");
        exit(EXIT_FAILURE);
    }

    /* trying to connect to the remote address. */
    printf("[+] Connecting...\n");
    if (connect(socket_client, client_address->ai_addr, client_address->ai_addrlen))
    {
        fprintf(stderr, "tcp client: connect() failed.\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(client_address); /* We don't worry about this anymore. */

    printf("[+] Connection has been established.\n");

    /* getting nickname from client. */
    printf("Enter nickname: ");
    char *nickname = (char *) malloc(sizeof(char) * 32);
    if (!nickname)
    {
        fprintf(stderr, "tcp client: malloc() failed.\n");
        exit(EXIT_FAILURE);
    }
    memset(nickname, '\0', sizeof(char) * 32);
    if (!fgets(nickname, sizeof(char) * 31, stdin))
    {
        fprintf(stderr, "tcp client: fgets() failed.\n");
        exit(EXIT_FAILURE);
    }
    for (int z = 0; z < strlen(nickname); z++)
    {
        if (nickname[z] == '\n')
            nickname[z] = '\0';
    }
    printf("Username for client @ %s is [%s]\n", address_buffer, nickname);
    /* This is a 'one-time thing'. */
    int sent_nick = send(socket_client, nickname, strlen(nickname), 0);
    if (sent_nick == -1)
    {
        fprintf(stderr, "tcp client: send() failed.\n");
        exit(EXIT_FAILURE);
    }

    /* main select-accept loop. */
    while(1)
    {
        fd_set reads;
        FD_ZERO(&reads); /* zero out reads. */
        FD_SET(socket_client, &reads);
        FD_SET(0, &reads);
        
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        /* select() requires that we pass a number that's larger than the largest socket discriptor we are going to monitor. */
        /* we use select() instead of fork(). */
        if (select(socket_client+1, &reads, 0, 0, &timeout) == -1)
        {
            fprintf(stderr, "tcp client: select() failed.\n");
            exit(EXIT_FAILURE);
        }

        /* after select() returns, we check whether our socket is set in <reads>. */
        if (FD_ISSET(socket_client, &reads))
        {
            char read[4096] = {'\0'};
            int bytes_received = recv(socket_client, read, sizeof(char) * 4096, 0);
            if (bytes_received == -1)
            {
                fprintf(stderr, "tcp client: recv() failed.\n");
                exit(EXIT_FAILURE);
            }
            if (bytes_received < 1)
            {
                printf("[!!] Connection closed by peer.\n");
                break;
            }
            printf("[+] Received (%d bytes): %.*s", bytes_received, bytes_received, read);
        }
        
        /* Now, we also need to check for terminal input. */
        if (FD_ISSET(0, &reads))
        {
            char read[4096] = {'\0'};
            if (fgets(read, sizeof(char) * 4096, stdin) == NULL) break;
            printf("[+] Sending: %s\n", read);
            int bytes_sent = send(socket_client, read, strlen(read), 0);
            printf("[+] Sent %d of %d bytes.\n", bytes_sent, (int) strlen(read));
        }
    }
    printf("[+] Closing socket...\n");
    CLOSESOCKET(socket_client);
    printf("[+] Done.\n");

    return 0;
}
