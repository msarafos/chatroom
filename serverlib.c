#include "serverlib.h"

void config_local_addr(const char *host, const char *port, 
struct addrinfo *hints, struct addrinfo **res)
{
    printf("[+] Configuring local address ...\n");
    sleep(2);
    memset(hints, 0, sizeof(*hints));
    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE; /* fill in host's IP address. */
    if (getaddrinfo(NULL, PORT, hints, &(*res)))
    {
        fprintf(stderr, "[!] server: getaddrinfo() failed.\n");
        exit(EXIT_FAILURE);
    }
    
}

SOCKET socket_creation(int domain, int socket_type, int protocol, struct addrinfo *hints)
{
    printf("[+] Socket creation ...\n");
    sleep(2);
    SOCKET socket_listen = socket(hints->ai_family, hints->ai_socktype, hints->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "[!] server: socket() failed.\n");
        exit(EXIT_FAILURE);
    }
    return socket_listen;
}

/* UNWANTED: hogging port: We want to resuse it in bind(). */
void server_options(int sockfd, int level, int optname, void *optval, socklen_t optlen)
{
    printf("[+] Setting options for the server socket ...\n");
    sleep(2);
    if (setsockopt(sockfd, level, optname, optval, optlen) < 0)
    {
        fprintf(stderr, "[!] server: setsockopt() failed.\n");
        exit(EXIT_FAILURE);
    }
}

void bind_to_addr(int sockfd, struct addrinfo *bind_address)
{
    printf("[+] Binding the socket to the local address ...\n");
    sleep(2);
    if (bind(sockfd, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "[!] server: bind() failed.\n");
        exit(EXIT_FAILURE);
    }
}

void listening(int sockfd, int backlog)
{
    printf("[+] Listening ...\n");
    sleep(2);
    printf("[+] Waiting for incoming connections ...\n");

    if (listen(sockfd, backlog))
    {
        fprintf(stderr, "[!] server: listen() failed.\n");
        exit(EXIT_FAILURE);
    }
}

/* initializing the client list. */
void init_list(CLIENT_NODE **root) { *root = NULL; }

/* searching for a specific nickname in the database. */
int find_client(CLIENT_NODE *root, char *nickname)
{
    CLIENT_NODE *tmp;
    for (tmp = root; tmp != NULL; tmp = tmp->next)
    {
        if (!strncmp(tmp->nickname, nickname, strlen(nickname)))
            return 1;
    }
    return 0;
}

/* Appending client to the list (inserting at the end). */
int append_client(CLIENT_NODE **root, char *IP, char *nickname)
{
    CLIENT_NODE *new_node = (CLIENT_NODE *) malloc(sizeof(CLIENT_NODE));
    if (!new_node)
    {
        fprintf(stderr, "[!] server: malloc: malloc() failed.\n");
        return -1;
    }

    /* check for already existing nickname in the database. */
    if (find_client(*root, nickname))
        return -1;

    /* insertion of data. */
    strncpy(new_node->IP, IP, strlen(IP));
    strncpy(new_node->nickname, nickname, strlen(nickname));

    if (*root == NULL) /* empty list. */
    {
        *root = new_node;
        new_node->previous = NULL;
        new_node->next = NULL;
        return 0;
    }
    CLIENT_NODE *tmp = *root;
    while (tmp->next != NULL)
        tmp = tmp->next;
    
    /* Now we point to the last node of the list. */
    tmp->next = new_node;
    new_node->previous = tmp;
    new_node->next = NULL;
    return 0;
}

/* Removing client from the server list. */
int remove_client(CLIENT_NODE **root, char *nickname)
{
	CLIENT_NODE *tmp = NULL;
	for (tmp = *root; tmp != NULL; tmp = tmp->next) 
	{
		if (tmp == *root && !(strcmp(tmp->nickname, nickname)))
		{
			*root = tmp->next;
			tmp->next->previous = NULL;
			free(tmp);
		}
		if (!(strcmp(tmp->nickname, nickname)))
		{
			tmp->previous->next = tmp->next;
			tmp->next->previous = tmp->previous;
			free(tmp);
			break;
		}
	}
	return 1; /* success. */
}


/* For debugging purposes. */
void print_list(CLIENT_NODE *root)
{
    printf("\nList of clients:\n");
    CLIENT_NODE *tmp = root;
    while (tmp != NULL)
    {
        printf("++++++++++++++++++++\n");
        printf("IP:       %s\n", tmp->IP);
        printf("nickname: %s\n", tmp->nickname);
        printf("++++++++++++++++++++\n");
        tmp = tmp->next;
    }
}
