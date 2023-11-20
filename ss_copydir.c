#include "headers.h"

void makeFolder(char* buffer_nm, int nm_sockfd)
{
    char* token = strtok(buffer_nm, " ");
    token = strtok(NULL, " ");
    if(mkdir(token, 0777) < 0)
    {
        if(send(nm_sockfd, "-1", strlen("-1"), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]Directory create error");
        exit(1);
    }
}

void fileBanao(char* buffer_nm, int nm_sockfd)
{
    char* token = strtok(buffer_nm, " ");
        token = strtok(NULL, " ");
        FILE* fd = fopen(token, "w+");
        bzero(buffer_nm, 1024);
        if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Recv error");
            exit(1);
        }
        while((strncmp(buffer_nm, "create_file", strlen("create_file")) != 0) && strncmp(buffer_nm, "create_folder", strlen("create_folder")) != 0)
        {
            fprintf(fd, "%s", buffer_nm);
            bzero(buffer_nm, 1024);
            if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
            {
                perror("[-]Recv error");
                exit(1);
            }
        }
        fclose(fd);
        if(strcmp(buffer_nm, "\n") == 0)
        {
            bzero(buffer_nm, 1024);
            strcpy(buffer_nm, "1");
            if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
            {
                perror("[-]Send error");
                exit(1);
            }
            return;
        }
        else if(strncmp(buffer_nm, "create_folder", strlen("create_folder")) == 0)
            makeFolder(buffer_nm, nm_sockfd);
        else if(strnmcp(buffer_nm, "create_file", strlen("create_file")) == 0)
            fileBanao(buffer_nm, nm_sockfd);
}

void recvDirFromSS(char* dir, char* dest, int nm_sockfd)
{
    int ack = -1;
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        exit(1);
    }
    if(strcmp(buffer_nm, "\n") == 0)
    {
        bzero(buffer_nm, 1024);
        strcpy(buffer_nm, "1");
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        return;
    }
    else if(strncmp(buffer_nm, "create_file", strlen("create_file")) == 0)
        fileBanao(buffer_nm, nm_sockfd);
    else if(strnmcp(buffer_nm, "create_folder", strlen("create_folder")) == 0)
        makeFolder(buffer_nm, nm_sockfd);
}

void filesender(char* file, char* dir, int nm_sockfd)
{
    char buffer[1024];
    bzero(buffer, 1024);

    char file_name[1024];
    char temp[1024];
    strcpy(temp, file);
    char* token = strtok(temp, "/");
    while(token != NULL)
    {
        bzero(file_name, 1024);
        strcpy(file_name, token);
        token = strtok(NULL, "/");
    }

    sprintf(buffer, "create_file %s/%s", dir, file_name);
    if(send(nm_sockfd, buffer, sizeof(buffer), 0) < 0)
    {
        perror("[-]Send error");
        exit;
    }

    FILE* fd = fopen(file, "r");
    bzero(buffer, 1024);
    while(fgets(buffer, sizeof(buffer), fd) != NULL)
    {
        if(send(nm_sockfd, buffer, sizeof(buffer), 0) < 0)
        {
            perror("[-]Send error");
            exit;
        }
        bzero(buffer, 1024);
    }
}

void sendDirToSS(char* dir, char* dest, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);

    DIR* dirp = opendir(dir);
    if(dirp == NULL)
    {
        if(send(nm_sockfd, "-1", strlen("-1"), 0) < 0)
        {
            perror("[-]Send error");
            return;
        }
        perror("[-]Directory open error");
        exit(1);
    }

    char dir_name[1024];
    char temp[1024];
    strcpy(temp, dir);
    char* token = strtok(temp, "/");
    while(token != NULL)
    {
        bzero(dir_name, 1024);
        strcpy(dir_name, token);
        token = strtok(NULL, "/");
    }

    char* new_dir = (char*)malloc(sizeof(char) * (strlen(dest) + strlen(dir) + 5));
    bzero(new_dir, sizeof(new_dir));
    sprintf(new_dir, "%s/%s", dest, dir_name);
    sprintf(buffer_nm, "create_folder %s", new_dir);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        return;
    }

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
            sendDirToSS(path, new_dir, nm_sockfd);
        else
            filesender(path, new_dir, nm_sockfd);
    }
}

void recursivelySend(char* dir, char* dest, int nm_sockfd)
{
    sendDirToSS(dir, dest, nm_sockfd);
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    strcpy(buffer_nm, "\n");
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        exit(1);
    }
}