#include "headers.h"

extern trie *root;
extern lru_head *head;
extern ss_info *array_of_ss_info;

// receive action from client
// for read: read filename
// for write: write filename and then after receiving ack write data
// for retrieve information: retrieve filename
// for create: create file filepath
// for delete: delete filename with path
// for copy file: copy filepath to newfilepath
// for create folder: create folder folderpath
// for delete folder: delete folder folderpath
// for copy folder: copy folder folderpath to newfolderpath

int what_to_do(char *input, int nm_sock_for_client)
{
    printf("input: %s\n", input);
    if ((strncmp(input, "read", strlen("read")) == 0) || (strncmp(input, "retrieve", strlen("retrieve")) == 0))
    {
        char *filename = strtok(input, " ");
        filename = strtok(NULL, " ");
        printf("filename: %s\n", filename);
        lru_node *node_in_cache = find_and_return(filename, head);
        if (node_in_cache == NULL)
        {
            printf("not in cache\n");
            // find in trie
            int ss_num = search(root, filename);
            printf("ss_num: %d\n", ss_num);
            if (ss_num == 0)
            {
                // send -1 as acknm_sock_for_client
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 0;
            }
            else
            {
                printf("found in trie\n");
                char send_details_to_client[BUF_SIZE];
                int port = array_of_ss_info[ss_num].ss_client_port;
                strcpy(send_details_to_client, array_of_ss_info[ss_num].ss_ip);

                // concatenate ip and port as a string separated by space
                char port_as_string[100];
                sprintf(port_as_string, " %d", port);
                strcat(send_details_to_client, port_as_string);

                printf("whats sent to client: %s#\n", send_details_to_client);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
            }
        }
        else
        {
            char send_details_to_client[BUF_SIZE];
            int port = node_in_cache->storage_server_port_for_client;
            char ss_ip[21];
            strcpy(ss_ip, node_in_cache->storage_server_ip);
            // concatenate ip and port as a string separated by space
            sprintf(send_details_to_client, "%s %d", ss_ip, port);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
        }
    }
    else if (strncmp(input, "write", strlen("write")) == 0)
    {
        // command: write filepath
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        // Check if the file is in the LRU cache
        lru_node *node_in_cache = find_and_return(file_path, head);
        // File not found in cache
        if (node_in_cache == NULL)
        {
            printf("not in cache\n");
            // Search in trie
            int ss_num = search(root, file_path);
            printf("ss_num: %d\n", ss_num);

            if (ss_num == 0)
            {
                // If file not found in trie, send -1 as acknowledgment to the client
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 0;
            }
            else
            {
                printf("found in trie\n");
                // File found in trie, send storage server details to the client
                char send_details_to_client[BUF_SIZE];
                int port = array_of_ss_info[ss_num].ss_client_port;
                strcpy(send_details_to_client, array_of_ss_info[ss_num].ss_ip);

                // Concatenate IP and port as a string separated by space
                char port_as_string[100];
                sprintf(port_as_string, " %d", port);
                strcat(send_details_to_client, port_as_string);

                printf("whats sent to the client: %s#\n", send_details_to_client);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
            }
        }
        // File found in the LRU cache
        else
        {
            // Send storage server details to the client
            char send_details_to_client[BUF_SIZE];
            int port = node_in_cache->storage_server_port_for_client;
            char ss_ip[21];
            strcpy(ss_ip, node_in_cache->storage_server_ip);

            // Concatenate IP and port as a string separated by space
            sprintf(send_details_to_client, "%s %d", ss_ip, port);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
        }
    }

    else if (strncmp(input, "create_file", strlen("create")) == 0)
    {
        // if file already exists send ack as -1
        // choose ss number by comparing string by removing file name
        // make connection with storage server and send action
        // receive ack from storage server
        // add to trie the file and storage server number
        // add to lru cache

        // command = create_file filepath
        // Get the file path from the command
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        // Checking if the file already exists in the trie
        if (search(root, file_path) > 0)
        {
            // Send -1 acknowledgment to the client
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", -1);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            return 0;
        }
        else
        {
            // Extract directory path
            char dir_path[1024];
            strcpy(dir_path, file_path);

            char *remove_file_name = strrchr(dir_path, '/');
            if (remove_file_name != NULL)
                *remove_file_name = '\0';

            // Find Storage Server number
            int ss_num = search(root, dir_path);

            // Handle directory not found
            if (ss_num == 0)
            {
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 0;
            }

            // Retrieve Storage Server information
            int ss_client_port = array_of_ss_info[ss_num].ss_client_port;
            int ss_nm_port = array_of_ss_info[ss_num].ss_nm_port;
            char *ss_ip = array_of_ss_info[ss_num].ss_ip;

            // Make a connection with the Storage Server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buffer[1024];

            // Create socket
            sock2 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr2, 0, sizeof(serv_addr2));
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = inet_addr(ss_ip);
            serv_addr2.sin_port = htons(ss_nm_port);

            // Connect to Storage Server
            if (connect(sock2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            // Send input to Storage Server
            if (send(sock2, input, strlen(input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive Acknowledgement from SS (waiting for Agrim)

            // Insert into trie
            insert(root, file_path, ss_num);

            // Insert into LRU
            lru_node *new_lru_node = make_lru_node(file_path, ss_num, ss_client_port, ss_ip);
            insert_at_front(new_lru_node, head);

            // Send Ack to client that opeartion is successfull

            close(sock2);
        }
    }

    else if (strncmp(input, "delete_file", strlen("delete")) == 0)
    {
        // search filepath in the trie
        // retireve ss_num and delete from ss_info and receive ack from ss
        // if found delete from trie
        // if present in lru delete from there
        // send ack to client

        // command = delete_file filepath
        // Get the file path from the command
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        int ss_num = search(root, file_path);

        if (ss_num <= 0)
        {
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", -1);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            return 0;
        }
        else if (ss_num > 0)
        {
            // Retrieve Storage Server information
            int ss_client_port = array_of_ss_info[ss_num].ss_client_port;
            int ss_nm_port = array_of_ss_info[ss_num].ss_nm_port;
            char *ss_ip = array_of_ss_info[ss_num].ss_ip;

            // Make a connection with the Storage Server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buffer[1024];

            // Create socket
            sock2 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr2, 0, sizeof(serv_addr2));
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = inet_addr(ss_ip);
            serv_addr2.sin_port = htons(ss_nm_port);

            // Connect to Storage Server
            if (connect(sock2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            // Send input to Storage Server
            if (send(sock2, input, strlen(input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive Ack from Storage Server

            close(sock2);
            // Delete the file path from the trie
            delete_node(root, file_path);
            // Search and delete from the LRU cache
            lru_node *deleted_node = delete_lru_node(file_path, head);
            free(deleted_node);
        }
    }
    else if (strncmp(input, "copy_file", strlen("copy")) == 0)
    {
    }
    else if (strncmp(input, "create_folder", strlen("create folder")) == 0)
    {
    }
    else if (strncmp(input, "delete_folder", strlen("delete folder")) == 0)
    {
    }
    else if (strncmp(input, "copy_folder", strlen("copy folder")) == 0)
    {
    }
    else
    {
        printf("Invalid action\n");
        return 0;
    }
}