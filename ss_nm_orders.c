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

void copyFile(char* file, char* dir, int nm_sockfd)
{
    int ack = -1;
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);

    char file_name[1024];
    char* token = strtok(file, "/");
    while(token != NULL)
    {
        bzero(file_name, 1024);
        strcpy(file_name, token);
        token = strtok(NULL, "/");
    }

    char* new_file = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(file) + 5));
    sprintf(new_file, "%s/%s", dir, file_name);
    if(strcmp(file, new_file) == 0)
    {
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            return;
        }
        perror("[-]File copy error");
        return;
    }

    FILE* fd1 = fopen(file, "r");
    FILE* fd2 = fopen(new_file, "w+");
    if(fd1 == NULL || fd2 == NULL)
    {
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]File open error");
        return;
    }
    char ch;
    while((ch = fgetc(fd1)) != EOF)
        fputc(ch, fd2);
    fclose(fd1);
    fclose(fd2);
    ack = 1;
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
}

void copyDir(char* dir, char* dest, int nm_sockfd)
{
    int ack = -1;
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    
    DIR* dirp = opendir(dir);
    if(dirp == NULL)
    {
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            return;
        }
        perror("[-]Directory open error");
        return;
    }

    char* new_dir = (char*)malloc(sizeof(char) * (strlen(dest) + strlen(dir) + 5));
    sprintf(new_dir, "%s/%s", dest, dir);
    if(strcmp(dir, new_dir) == 0)
    {
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            return;
        }
        perror("[-]Folder copy error");
        return;
    }
    make_dir(new_dir, nm_sockfd);

    struct dirent* entry = readdir(dirp);
    struct stat statbuff;
    while(entry != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            entry = readdir(dirp);
            continue;
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if(stat(path, &statbuff) < 0)
        {
            perror("[-]File stat error");
            continue;
        }
        if(S_ISDIR(statbuff.st_mode))
            codyDir(path, new_dir, nm_sockfd);
        else
            copyFile(path, new_dir, nm_sockfd);
        entry = readdir(dirp);
    }
    ack = 1;
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        return;
    }
}