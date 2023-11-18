#include "ss_nm_orders.h"

void make_file(char* file, char* buffer_nm, int nm_sockfd)
{
    int fd = open(file, O_CREAT, 0666);
    sprintf(buffer_nm, "%d", fd);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    if(fd < 0)
    {
        perror("[-]File create error");
        exit(1);
    }
    close(fd);
}

void del_file(char* file, char* buffer_nm, int nm_sockfd)
{
    int ack = remove(file);
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    if(ack < 0)
    {
        perror("[-]File delete error");
        exit(1);
    }
}

void delete_dir(char* name, char* buffer_nm, int nm_sockfd)
{
    DIR* dir = opendir(name);
    if(dir == NULL)
    {
        perror("[-]Directory delete error");
        exit(1);
    }
    struct dirent* entry = readdir(dir);
    struct stat statbuff;
    while(entry != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            entry = readdir(dir);
            continue;
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if(stat(path, &statbuff) < 0)
        {
            perror("[-]File stat error");
            continue;
        }
        if(S_ISDIR(statbuff.st_mode))
            delete_dir(path, buffer_nm, nm_sockfd);
        else
        {
            int ack = remove(path);
            sprintf(buffer_nm, "%d", ack);
            if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
            {
                perror("[-]Send error");
                exit(1);
            }
            if(ack < 0)
            {
                perror("[-]File delete error");
                exit(1);
            }
        }
    }
}

void make_dir(char* name, char* buffer_nm, int nm_sockfd)
{
    int ack = mkdir(name, 0777);
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    if(ack < 0)
    {
        perror("[-]Directory create error");
        exit(1);
    }
}