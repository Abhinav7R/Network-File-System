#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>

#define PORT 4545

char* Read = "read";
char* Write = "write";
char* Retrieve = "retrieve";

typedef struct filepaths* files;
typedef struct filepaths
{
    char* name;
    files next;
}filepaths;

typedef struct threadargs* args;
typedef struct threadargs
{
    socklen_t addr_size;
    struct sockaddr_in server_addr;
    int server_sock;
    int sockfd;
    char* buffer;
}threadargs;

void* nm_handler(char* buffer_nm, int nm_sockfd, int port, int client_port, char* ip, char* paths_of_all)
{
    bzero(buffer_nm, 1024);
    sscanf(buffer_nm, "IP: %s\nNM_PORT: %d\nCLIENT PORT: %d\n", ip, &port, &client_port);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Receive error");
        exit(1);
    }
    if(send(nm_sockfd, paths_of_all, sizeof(paths_of_all), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Receive error");
        exit(1);
    }

    close(nm_sockfd);
}

void* nm_handler_for_ops(void* arg)
{
    args args_nm = (args)arg;
    socklen_t addr_size = args_nm->addr_size;
    struct sockaddr_in server_addr_1 = args_nm->server_addr;
    int server_sock_1 = args_nm->server_sock;
    int nm_sockfd = args_nm->sockfd;
    char* buffer_nm = args_nm->buffer;

    addr_size = sizeof(server_addr_1);
    nm_sockfd = accept(server_sock_1, (struct sockaddr*)&server_addr_1, &addr_size);
    if(nm_sockfd < 0)
    {
        perror("[-]Accept error");
        exit(1);
    }
    printf("[+]Naming server connected.\n");

    while(1)
    {
        bzero(buffer_nm, 1024);
        if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
    }

    pthread_exit(NULL);
}

void* client_handler(void* arg)
{
    args args_client = (args)arg;
    socklen_t addr_size = args_client->addr_size;
    struct sockaddr_in server_addr_2 = args_client->server_addr;
    int server_sock_2 = args_client->server_sock;
    int client_sockfd = args_client->sockfd;
    char* buffer_client = args_client->buffer;

    addr_size = sizeof(server_addr_2);
    client_sockfd = accept(server_sock_2, (struct sockaddr*)&server_addr_2, &addr_size);
    if(client_sockfd < 0)
    {
        perror("[-]Accept error");
        exit(1);
    }
    printf("[+]Client connected.\n");
    
    while(1)
    {
        bzero(buffer_client, 1024);
        if(recv(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        if(strncmp(buffer_client, Read, strlen(Read)) == 0)
        {
            char* token = strtok(buffer_client, " ");
            token = strtok(NULL, " ");
            char* file = token;
            FILE* fd = fopen(file, "rb");
            if(fd == NULL)
            {
                perror("[-]File open error");
                exit(1);
            }
            fseek(fd, 0, SEEK_END);
            long size = ftell(fd);
            fseek(fd, 0, SEEK_SET);
            fclose(fd);
            long num_packets = size/1024;
            if(size%1024 != 0)
                num_packets++;
            bzero(buffer_client, 1024);
            sprintf(buffer_client, "%ld", num_packets);
            if(send(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
            {
                perror("[-]Send error");
                exit(1);
            }
            if(open(file, O_RDONLY) < 0)
            {
                perror("[-]File open error");
                exit(1);
            }
            while(num_packets--)
            {
                bzero(buffer_client, 1024);
                if(send(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
                {
                    perror("[-]Send error");
                    exit(1);
                }
            }
            fclose(fd);
        }
        else if(strncmp(buffer_client, Write, sizeof(Write)) == 0)
        {
            char* token = strtok(buffer_client, " ");
            token = strtok(NULL, " ");
            char* file = token;
            FILE* fd = fopen(file, "wb");
            if(fd == NULL)
            {
                perror("[-]File open error");
                exit(1);
            }
            while(1)
            {
                bzero(buffer_client, 1024);
                if(recv(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
                {
                    perror("[-]Receive error");
                    exit(1);
                }
                if(strcmp(buffer_client, "\n") == 0)
                    break;
                fprintf(fd, "%s", buffer_client);
                fprintf(fd, "\n");
            }
            fclose(fd);
        }
        else if(strncmp(buffer_client, Retrieve, strlen(Retrieve)) == 0)
        {
            char* token = strtok(buffer_client, " ");
            token = strtok(NULL, " ");
            char* file = token;
            struct stat fileStat;
            if(stat(file, &fileStat) < 0)
            {
                perror("[-]File stat error");
                exit(1);
            }
            bzero(buffer_client, 1024);
            //concatenate file size and permissions into buffer
            sprintf(buffer_client, "%ld %o", fileStat.st_size, fileStat.st_mode);
            if(send(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
            {
                perror("[-]Send error");
                exit(1);
            }
        }
    }
    close(client_sockfd);
    pthread_exit(NULL);
}


//storage server implementation for NFS
int main(int argc, char *argv[])
{
    // if(argc != 3)
    // {
    //     printf("Usage: %s <nm_port> <client_port>\n", argv[0]);
    //     exit(1);
    // }
    
    char* ip = "127.0.0.1";
    // int nm_port = atoi(argv[2]);
    // int client_port = atoi(argv[3]);
    int nm_port = 4545;
    int client_port = 7070;

    printf("IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\nEnter file paths(relative to current directory) that the clients can access: (Enter 'DONE' when finished)\n", ip, nm_port, client_port);

    files head = (files)malloc(sizeof(filepaths));
    files temp = head;
    while(1)
    {
        char* path = (char*)calloc(sizeof(char), 100);
        scanf("%s", path);
        if(strcmp(path, "DONE") == 0)
            break;
        temp->name = (char*)malloc(sizeof(char)*100);
        strcpy(temp->name, path);
        temp->next = (files)malloc(sizeof(filepaths));
        temp = temp->next;
    }

    char* paths_of_all = (char*)malloc(sizeof(char)*60000);
    strcpy(paths_of_all, head->name);
    head = head->next;
    while(head->name != NULL)
    {
        strcat(paths_of_all, "|");
        strcat(paths_of_all, head->name);
        head = head->next;
    }
    strcat(paths_of_all, "|");
    
    int socky, server_sock_1, server_sock_2, nm_sockfd, client_sockfd;
    struct sockaddr_in server_addr_1, server_addr_2, nm_addr, cliett_addr;
    socklen_t addr_size;
    char buffer_nm[1024], buffer_client[1024];

    socky = socket(PF_INET, SOCK_STREAM, 0);
    if(server_sock_1 < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    // printf("[+]TCP server socket created.\n");

    memset(&server_addr_1, '\0', sizeof(server_addr_1));
    server_addr_1.sin_family = AF_INET;
    server_addr_1.sin_port = htons(PORT);
    server_addr_1.sin_addr.s_addr = inet_addr(ip);

    if(connect(server_sock_1, (struct sockaddr*)&server_addr_1, sizeof(server_addr_1)) == -1)
    {
        perror("connect() error");
        exit(1);
    }
    nm_handler(buffer_nm, socky, PORT, client_port, ip, paths_of_all);

    //connecting to client
    server_sock_2 = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_sock_2 < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    memset(&server_addr_2, '\0', sizeof(server_addr_2));
    server_addr_2.sin_family = AF_INET;
    server_addr_2.sin_port = htons(client_port);
    server_addr_2.sin_addr.s_addr = inet_addr(ip);
    if(bind(server_sock_2, (struct sockaddr*)&server_addr_2, sizeof(server_addr_2)) < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    
    //connecting to naming server
    server_sock_1 = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_sock_1 < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    memset(&server_addr_1, '\0', sizeof(server_addr_1));
    server_addr_1.sin_family = AF_INET;
    server_addr_1.sin_port = htons(nm_port);
    server_addr_1.sin_addr.s_addr = inet_addr(ip);
    if(bind(server_sock_1, (struct sockaddr*)&server_addr_1, sizeof(server_addr_1)) < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }

    //separate threads for clients and naming server
    pthread_t input[2];
    addr_size = sizeof(server_addr_1);
    args args_nm = (args)malloc(sizeof(threadargs));
    args_nm->addr_size = addr_size;
    args_nm->server_addr = server_addr_1;
    args_nm->server_sock = server_sock_1;
    args_nm->sockfd = nm_sockfd;
    args_nm->buffer = buffer_nm;
    nm_handler_for_ops((void*)args_nm);
    if(pthread_create(&input[0], NULL, nm_handler_for_ops, (void*)args_nm) != 0)
        printf("[-]Thread creation error.\n");

    addr_size = sizeof(server_addr_2);
    args args_client = (args)malloc(sizeof(threadargs));
    args_client->addr_size = addr_size;  
    args_client->server_addr = server_addr_2;
    args_client->server_sock = server_sock_2;
    args_client->sockfd = client_sockfd;
    args_client->buffer = buffer_client;
    if(pthread_create(&input[1], NULL, client_handler, (void*)args_client) != 0)
        printf("[-]Thread creation error.\n");

    close(server_sock_1);
    close(server_sock_2);
    return 0;
}