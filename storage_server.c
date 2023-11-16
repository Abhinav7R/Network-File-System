#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

typedef struct filepaths* files;
typedef struct filepaths
{
    char* name;
    files next;
}filepaths;


//storage server implementation for NFS
int main(int argc, char *argv[])
{
    if(argc != 4)
        printf("Usage: %s <ip> <nm_port> <client_port>\n", argv[0]);
    
    char* ip = (char*)calloc(sizeof(char), 21);
    strcpy(ip, argv[1]);
    int nm_port = atoi(argv[2]);
    int client_port = atoi(argv[3]);

    printf("IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\nEnter file paths(relative to current directory) that the clients can access: (Enter 'DONE' when finished)\n", ip, nm_port, client_port);

    files head = (files)malloc(sizeof(filepaths));
    files temp = head;
    while(1)
    {
        char* path = (char*)calloc(sizeof(char), 100);
        scanf("%s", path);
        if(strcmp(path, "DONE") == 0)
            break;
        temp->name = path;
        temp->next = (files)malloc(sizeof(filepaths));
        temp = temp->next;
    }
    
    int server_sock_1, server_sock_2, nm_sockfd, client_sockfd;
    struct sockaddr_in server_addr_1, server_addr_2, nm_addr, client_addr;
    socklen_t addr_size;
    char buffer_nm[1024], buffer_client[1024];

    server_sock_1 = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_sock_1 < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    memset(&server_addr_1, '\0', sizeof(server_addr_1));
    server_addr_1.sin_family = AF_INET;
    server_addr_1.sin_port = htons(nm_port);
    server_addr_1.sin_addr.s_addr = inet_addr(ip);

    int n = bind(server_sock_1, (struct sockaddr*)&server_addr_1, sizeof(server_addr_1));
    if(n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to port %d\n", nm_port);

    if(listen(server_sock_1, 5) < 0)
    {
        perror("[-]Listen error");
        exit(1);
    }
    addr_size = sizeof(nm_addr);
    nm_sockfd = accept(server_sock_1, (struct sockaddr*)&nm_addr, &addr_size);
    if(nm_sockfd < 0)
    {
        perror("[-]Accept error");
        exit(1);
    }
    printf("[+]Naming server connected.\n");

    bzero(buffer_nm, 1024);
    sscanf(buffer_nm, "IP: %s\nNM_PORT: %d\nCLIENT PORT: %d\n", ip, &nm_port, &client_port);
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
    while(1)
    {
        bzero(buffer_nm, 1024);
        if(head != NULL)
        {
            strcpy(buffer_nm, head->name);
            head = head->next;
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
        }
        else
            break;
    }

    bzero(buffer_nm, 1024);
    strcpy(buffer_nm, "Ready to receive");
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }

    while(1)
    {
        bzero(buffer_nm, 1024);
        if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        if(strcmp(buffer_nm, "READ") == 0)
        {
            bzero(buffer_nm, 1024);
            strcpy(buffer_nm, "Send name of file");
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
            char* file = (char*)calloc(sizeof(char), 100);
            strcpy(file, buffer_nm);
            
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
            int fd = open(file, O_RDONLY);
            if(fd < 0)
            {
                perror("[-]File open error");
                exit(1);
            }
            
            while(read(fd, buffer_client, 1024) > 0)
            {
                bzero(buffer_client, 1024);
                if(send(server_sock_2, buffer_client, sizeof(buffer_client), 0) < 0)
                {
                    perror("[-]Send error");
                    exit(1);
                }
                bzero(buffer_client, 1024);
                if(recv(server_sock_2, buffer_client, sizeof(buffer_client), 0) < 0)
                {
                    perror("[-]Receive error");
                    exit(1);
                }
            }
            close(fd);
            close(server_sock_2);    
        }
        else if(strcmp(buffer_nm, "WRITE") == 0)
        {
            bzero(buffer_nm, 1024);
            strcpy(buffer_nm, "Send name of file");
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
            char* file = (char*)calloc(sizeof(char), 100);
            strcpy(file, buffer_nm);

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
            int fd = open(file, O_APPEND | O_WRONLY | O_CREAT, 0644);
            if(fd < 0)
            {
                perror("[-]File open error");
                exit(1);
            }

            while(1)
            {
                bzero(buffer_client, 1024);
                if(recv(server_sock_2, buffer_client, sizeof(buffer_client), 0) < 0)
                {
                    perror("[-]Receive error");
                    exit(1);
                }
                if(strcmp(buffer_client, "EOF") == 0)
                    break;
                lseek(fd, 0, SEEK_END);
                write(fd, buffer_client, strlen(buffer_client));
                bzero(buffer_client, 1024);
                strcpy(buffer_client, "Received");
                if(send(server_sock_2, buffer_client, sizeof(buffer_client), 0) < 0)
                {
                    perror("[-]Send error");
                    exit(1);
                }
            }
            close(fd);
            close(server_sock_2);
        }
        else if(strcmp(buffer_nm, "RETRIEVE") == 0)
        {
            bzero(buffer_nm, 1024);
            strcpy(buffer_nm, "Send name of file");
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
            char* file = (char*)calloc(sizeof(char), 100);
            strcpy(file, buffer_nm);

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
        }
        else if(strcmp(buffer_nm, "CREATE") == 0)
        {

        }
        else if(strcmp(buffer_nm, "DELETE") == 0)
        {

        }
        else if(strcmp(buffer_nm, "COPY") == 0)
        {

        }
        else if(strcmp(buffer_nm, "EXIT") == 0)
        {
            printf("[+]Exiting.\n");
            break;
        }
        else
        {
            printf("[-]Invalid command received.\n");
            break;
        }
    }
}