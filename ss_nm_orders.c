#include "ss_nm_orders.h"

void make_file(char* file, int nm_sockfd)
{
    // printf("create file %s\n", file);
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    // int fd = open(file, O_CREAT | O_RDWR, 0777);
    FILE* fd = fopen(file, "w+");
    if(fd != NULL)
    {
        bzero(buffer_nm, 1024);
        strcpy(buffer_nm, "1");
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
    }
    else
    {
        bzero(buffer_nm, 1024);
        strcpy(buffer_nm, "-1");
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]File create error");
        //error handling to be done
        //dont exit program but just return
        // exit(1);
        return;
    }
    // sprintf(buffer_nm, "%d", fd);
    // if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    // {
    //     perror("[-]Send error");
    //     exit(1);
    // }
    // if(fd < 0)
    // {
    //     perror("[-]File create error");
    //     //error handling to be done
    //     //dont exit program but just return
    //     exit(1);
    // }
    fclose(fd);
}

void del_file(char* file, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
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

void delete_dir(char* name, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
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
            delete_dir(path, nm_sockfd);
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
    rmdir(name);
}

void make_dir(char* name, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
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