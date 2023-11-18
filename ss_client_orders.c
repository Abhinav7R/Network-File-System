#include "ss_client_orders.h"

void read_file(char* file, int client_sockfd)
{
    char buffer_client[1024];
    FILE* fd = fopen(file, "rb");
    if(fd == NULL)
    {
        perror("[-]File open error");
        exit(1);
    }
    fseek(fd, 0, SEEK_END);
    long size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    printf("size: %ld\n", size);
    long num_packets = size/1024;
    if(size%1024 != 0)
        num_packets++;
    bzero(buffer_client, 1024);
    sprintf(buffer_client, "%ld", num_packets);
    if(send(client_sockfd, buffer_client, BUF_SIZE, 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    bzero(buffer_client, 1024);
    if(recv(client_sockfd, buffer_client, BUF_SIZE, 0) < 0)
    {
        perror("[-]Receive error");
        exit(1);
    }
    printf("received ack\n");
    while(num_packets--)
    {
        bzero(buffer_client, 1024);
        if(fread(buffer_client, 1, 1024, fd) < 0)
        {
            perror("[-]File read error");
            exit(1);
        }
        if(send(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        bzero(buffer_client, 1024);
        if(recv(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
    }
    fclose(fd);
}

void write_file(char* file, int client_sockfd)
{
    char buffer_client[1024];
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

void retrieve_info(char* file, int client_sockfd)
{
    char buffer_client[1024];
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