/* tcp_serve_chat.c */

#include "serverlib.h"

int main(int argc, char **argv)
{
    /* Configuring local address. */
    struct addrinfo hints, *bind_address = NULL;
    config_local_addr(NULL, PORT, &hints, &bind_address);

    /* Creating the server socket. */ 
    SOCKET socket_listen = socket_creation(hints.ai_family, hints.ai_socktype, hints.ai_protocol, &hints);

    /* UNWANTED: hogging port: We want to resuse it in bind(). */
    int option = 1;
    server_options(socket_listen, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    /* Binding. */
    bind_to_addr(socket_listen, bind_address);   
    freeaddrinfo(bind_address); /* no need to worry about this anymore. */

    /* Listening and waiting for clients. */
    listening(socket_listen, BACKLOG);


    /* We define a structure which stores all the active sockets. 
    We also maintain a max_socket variable, which holds the largest socket decriptor. */
    fd_set master; 
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    /* setting up the client list. */
    CLIENT_NODE *root;
    init_list(&root);

    while(1)
    {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) == -1)
        {
            fprintf(stderr, "tcp server: select() failed.\n");
            exit(EXIT_FAILURE);
        }

        SOCKET i;
        for (i = 1; i <= max_socket; i++)
        {
            if (FD_ISSET(i, &reads)) /* This means that our socket is IN the set. */
            {
                /* first, we need to find out which socket is the listening one. When we do, we call accept(). */
                if (i == socket_listen)
                {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(socket_listen, (struct sockaddr *) &client_address, &client_len);
                    if (!ISVALIDSOCKET(socket_client))
                    {
                        fprintf(stderr, "tcp server: accept() failed.\n");
                        exit(EXIT_FAILURE);
                    }
                    /* We need to place the new socket into the set. */
                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;
                    

                    /* Printing the IP address of the last connection. */
                    char address_buffer[128] = {'\0'};
                    getnameinfo((struct sockaddr *) &client_address, client_len, 
                    address_buffer, sizeof(address_buffer), 
                    0, 0, NI_NUMERICHOST);
                    printf("\n[+] Got connection from: %s\n", address_buffer);

                    /* we need to grab the client info and append it to the list. */
                    char client_nickname[32] = {'\0'};
                    int bytes_nickname = recv(socket_client, client_nickname, sizeof(char) * 31, 0);
                    if (bytes_nickname == -1)
                    {
                        fprintf(stderr, "[!] server: recv() failed.\n");
                        exit(EXIT_FAILURE);
                    }
                    if (bytes_nickname < 1)
                        printf("[!!] Connection terminated by client @ %s\n", address_buffer);
                    if (append_client(&root, address_buffer, client_nickname) < 0)
                    {
                        fprintf(stderr, "server: append_client() failed.\n");
                        char *error_msg = "[!] A client with this username already exists in the server. Try connecting with another one!\n";
                        int bytes_sent_error = send(socket_client, error_msg, strlen(error_msg), 0);
                        if (bytes_sent_error == -1)
                        {
                            fprintf(stderr, "[!] server: send() failed.\n");
                            exit(EXIT_FAILURE);
                        }
                        FD_CLR(socket_client, &master);
                        CLOSESOCKET(socket_client);
                        continue;
                    }
                    print_list(root);
                }
                else /* otherwise, it is a request from an established connection and we need to call recv(). */ 
                {
					
                    char read[1024] = {'\0'};
                    int bytes_received = recv(i, read, sizeof(char) * 1024, 0);
                    if (bytes_received < 1)
                    {
                        printf("\n[!!] No connection.\n");
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
						printf("The client list now is:\n");
						print_list(root);
                        continue;
                    }
                    SOCKET j;
                    for (j = 1; j <= max_socket; j++)
                    {
                        if (FD_ISSET(j, &master))
                        {
                            if (j == socket_listen || j == i) 
                                continue;
                            else 
                                send(j, read, bytes_received, 0);
                        }
                    }
                }
            }
        }
    }
    printf("[+] Closing listening socket...\n");
    CLOSESOCKET(socket_listen);
    printf("[+] Done.\n");

    return 0;
}
