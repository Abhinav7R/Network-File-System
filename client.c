#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 1024

//client program
int main()
{
    char *nm_ip = "127.0.0.1";    // ip address of Naming Server
    int nm_port = 1234;        // port number of Naming Server
    int sock;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    int read_cnt;

    // Clients initiate communication with the Naming Server (NM) to interact with the NFS

    // Create socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    // Initialize serv_addr
    memset(&serv_addr, 0, sizeof(serv_addr)); //init
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(nm_ip);
    serv_addr.sin_port = htons(nm_port);

    // Connect to server
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect() error");
        exit(1);
    }

    //receive ack from Naming Server
    //add timeout later
    if(recv(sock,buf,BUF_SIZE,0)<0)
    {
        perror("recv() error");
        exit(1);
    }
    
    //First send action to Naming Server 
    //actions are read write retrieve information are actions on files
    //create delete copy are actions on files and directories

    //send action to Naming Server take input from user
    //for read: read filename
    //for write: write filename and then after receiving ack write data
    //for retrieve information: retrieve filename
    //for create: create file filepath
    //for delete: delete filename with path
    //for copy file: copy filepath to newfilepath
    //for create folder: create folder folderpath
    //for delete folder: delete folder folderpath
    //for copy folder: copy folder folderpath to newfolderpath

    //for read write retrieve information connect to storage server
    //create delete copy receive ack from Naming Server

    while(1)
    {

        char input[BUF_SIZE];
        printf("Enter action: \n");
        fgets(input,100,stdin);

        if(strcmp(input,"exit")==0)
        {
            break;
        }

        if(send(sock,input,strlen(input),0)<0)
        {
            perror("send() error");
            exit(1);
        }

        //if input is read write or retrieve information then receive port and ip of storage server
        //connect to storage server and send action to storage server

        if(strncmp(input,"read",strlen("read"))==0 || strncmp(input,"write",strlen("write"))==0 || strncmp(input,"retrieve",strlen("retrieve"))==0)
        {
            char *server_ip = strtok(buf," ");
            char *server_port = strtok(NULL," ");
            printf("ip: %s port: %s\n",server_ip,server_port);

            //connect to storage server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buf2[BUF_SIZE];
            int read_cnt2;

            // Create socket
            sock2 = socket(PF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            // Initialize serv_addr
            memset(&serv_addr2, 0, sizeof(serv_addr2)); //init
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = inet_addr(server_ip);
            serv_addr2.sin_port = htons(atoi(server_port));

            // Connect to server
            if(connect(sock2, (struct sockaddr*)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            //if input is read then receive data from storage server
            //if input is write then send data to storage server
            //if input is retrieve information then receive information from storage server

            if(strncmp(input,"read",strlen("read"))==0)
            {
                //receive data from storage server
                //first receive number of packets as integer
                int num_packets;
                if(recv(sock2,&num_packets,sizeof(num_packets),0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                
                bzero(buf2,BUF_SIZE);
                //recv data from storage server
                while(num_packets--)
                {
                    if(recv(sock2,buf2,BUF_SIZE,0)<0)
                    {
                        perror("recv() error");
                        exit(1);
                    }
                    printf("%s",buf2);
                    bzero(buf2,BUF_SIZE);
                }
            }
            else if(strncmp(input,"write",strlen("write"))==0)
            {
                //send data to storage server
                char data[BUF_SIZE];
                printf("Enter data: \n");
                //scan data until 2 enters continuously
                while(1)
                {
                    //send line by line
                    //assume that each line is less than 1024 characters
                    //also send \n at end of data
                    fgets(data,BUF_SIZE,stdin);
                    
                    if(send(sock2,data,strlen(data),0)<0)
                    {
                        perror("send() error");
                        exit(1);
                    }
                    //if 2 enters then break
                    if(strcmp(data,"\n")==0)
                    {
                        break;
                    }
                }
            }
            else if(strncmp(input,"retrieve",strlen("retrieve"))==0)
            {
                //receive information from storage server
                while(1)
                {
                    bzero(buf2,BUF_SIZE);
                    if(recv(sock2,buf2,BUF_SIZE,0)<0)
                    {
                        perror("recv() error");
                        exit(1);
                    }
                    if(strcmp(buf2,"\n")==0)
                    {
                        break;
                    }
                    printf("%s",buf2);
                }
            }
        }
        else
        {   bzero(buf,BUF_SIZE);
            if(strncmp(input,"create",strlen("create"))==0)
            {
                //receive ack from naming server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
            }
            else if(strncmp(input,"delete",strlen("delete"))==0)
            {
                //receive ack from storage server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
            }
            else if(strncmp(input,"copy",strlen("copy"))==0)
            {
                //receive ack from storage server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
            }
            else if(strncmp(input,"create folder",strlen("create folder"))==0)
            {
                //receive ack from storage server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
            }
            else if(strncmp(input,"delete folder",strlen("delete folder"))==0)
            {
                //receive ack from storage server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
            }
            else if(strncmp(input,"copy folder",strlen("copy folder"))==0)
            {
                //receive ack from storage server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
            }
        
        }

    }


    
    

}