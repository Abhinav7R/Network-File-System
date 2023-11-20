    else if (strncmp(input, "copy_folder", strlen("copy_folder")) == 0)
    {
        //command copy_folder folderpath new_folderpath
        char temp[1024];
        strcpy(temp, input);

        char* old_folderpath;
        char* new_folderpath;

        //Tokenize the input to extract old_folderpath and new_folderpath
        old_folderpath = strtok(temp, " ");
        old_folderpath = strtok(NULL, " ");
        new_folderpath = strtok(NULL, " ");

        //Check if the folder is in the LRU cache
        lru_node* node_in_cache_1 = find_and_return(old_folderpath, head);
        lru_node* node_in_cache_2 = find_and_return(new_folderpath, head);
        int ss_num_1;
        int ss_num_2;
        int ss_client_port_1, ss_client_port_2;
        int ss_nm_port_1, ss_nm_port_2;
        char* ss_ip_1;
        char* ss_ip_2;
        int port_1, port_2;

        if(node_in_cache_1 == NULL)
        {
            //Search in trie
            ss_num_1 = search(root, old_folderpath);

            if(ss_num_1 == 0)
            {
                //If folder not found in trie, send -1 as acknowledgment to the client
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if(send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 0;
            }

            else
            {
                //Folder found in trie
                //Retrieve Storage Server information
                ss_client_port_1 = array_of_ss_info[ss_num_1].ss_client_port;
                ss_nm_port_1 = array_of_ss_info[ss_num_1].ss_nm_port;
                ss_ip_1 = array_of_ss_info[ss_num_1].ss_ip;
            }
        }
        // Folder found in the LRU cache
        else
        {
            port_1 = node_in_cache_1->storage_server_port_for_client;
            strcpy(ss_ip_1, node_in_cache_1->storage_server_ip);
        }

        if(node_in_cache_2 == NULL)
        {
            //Search in trie
            ss_num_2 = search(root, new_folderpath);

            if(ss_num_2 == 0)
            {
                //If folder not found in trie, send -1 as acknowledgment to the client
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if(send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 0;
            }

            else
            {
                //Folder found in trie
                //Retrieve Storage Server information
                ss_client_port_2 = array_of_ss_info[ss_num_2].ss_client_port;
                ss_nm_port_2 = array_of_ss_info[ss_num_2].ss_nm_port;
                ss_ip_2 = array_of_ss_info[ss_num_2].ss_ip;
            }
        }
        //Folder found in the LRU cache
        else
        {
            //Retrieve Storage Server information
            port_2 = node_in_cache_2->storage_server_port_for_client;
            strcpy(ss_ip_2, node_in_cache_2->storage_server_ip);
        }
        if(ss_num_1 == ss_num_2)
        {
            //Make a connection with the Storage Server
            int sock1;
            struct sockaddr_in serv_addr1;
            char buffer1[BUF_SIZE];

            //Create socket
            sock1 = socket(AF_INET, SOCK_STREAM, 0);
            if(sock1 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr1, 0, sizeof(serv_addr1));
            serv_addr1.sin_family = AF_INET;
            serv_addr1.sin_addr.s_addr = inet_addr(ss_ip_1);
            serv_addr1.sin_port = htons(ss_nm_port_1);

            //Connect to Storage Server
            if(connect(sock1, (struct sockaddr*)&serv_addr1, sizeof(serv_addr1)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            //Send input and "same" to Storage Server
            char combined_input[BUF_SIZE];
            sprintf(combined_input, "%s same", input);
            if(send(sock1, combined_input, strlen(combined_input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            //Receive acknowledgment from Storage Server
            int ack;
            char buffer_ack[BUF_SIZE];
            if(recv(sock1, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            //Close the connection to Storage Server
            close(sock1);

            //Send the same acknowledgment to the client
            if(send(nm_sock_for_client, buffer_ack, strlen(buffer_ack), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
        }
        else
        {
            int sock_ss1, sock_ss2;
            struct sockaddr_in serv_addr_ss1, serv_addr_ss2;
            char buffer_ss1[BUF_SIZE], buffer_ss2[BUF_SIZE];

            //Create sockets
            sock_ss1 = socket(AF_INET, SOCK_STREAM, 0);
            sock_ss2 = socket(AF_INET, SOCK_STREAM, 0);

            if(sock_ss1 == -1 || sock_ss2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            //Setup connection details for Storage Server 1
            memset(&serv_addr_ss1, 0, sizeof(serv_addr_ss1));
            serv_addr_ss1.sin_family = AF_INET;
            serv_addr_ss1.sin_addr.s_addr = inet_addr(ss_ip_1);
            serv_addr_ss1.sin_port = htons(ss_nm_port_1);

            //Setup connection details for Storage Server 2
            memset(&serv_addr_ss2, 0, sizeof(serv_addr_ss2));
            serv_addr_ss2.sin_family = AF_INET;
            serv_addr_ss2.sin_addr.s_addr = inet_addr(ss_ip_2);
            serv_addr_ss2.sin_port = htons(ss_nm_port_2);

            //Connect to Storage Server 1
            if(connect(sock_ss1, (struct sockaddr*)&serv_addr_ss1, sizeof(serv_addr_ss1)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            //Connect to Storage Server 2
            if(connect(sock_ss2, (struct sockaddr*)&serv_addr_ss2, sizeof(serv_addr_ss2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            char input_to_ss1[BUF_SIZE];
            sprintf(input_to_ss1, "%s send", input);
            //Send input to Storage Server 1
            if(send(sock_ss1, input_to_ss1, strlen(input_to_ss1), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            char input_to_ss2[BUF_SIZE];
            sprintf(input_to_ss2, "%s receive", input);
            //Send input to Storage Server 2
            if(send(sock_ss2, input_to_ss2, strlen(input_to_ss2), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            //Start the process
            char bufferFor1[BUF_SIZE], bufferFor2[BUF_SIZE];
            while(1)
            {
                if(recv(sock_ss1, bufferFor1, sizeof(bufferFor1), 0) < 0)
                {
                    perror("recv() error");
                    exit(1);
                }
                if(strcmp(bufferFor1, "\n") == 0)
                    break;
                else if((strncmp(bufferFor1, "create_folder", strlen("create_folder")) == 0) || (strncmp(bufferFor1, "create_file", strlen("create_file")) == 0))
                {
                    char temp[BUF_SIZE];
                    strcpy(temp, bufferFor1);
                    char* token_ss1 = strtok(temp, " ");
                    token_ss1 = strtok(NULL, " ");

                    char filepath_ss1[BUF_SIZE];
                    strcpy(filepath_ss1, token_ss1);
                    printf("filepath: %s\n", filepath_ss1);

                    //Insert into trie
                    insert(root, filepath_ss1, ss_num_2);

                    //Insert into LRU
                    lru_node* new_lru_node = make_lru_node(filepath_ss1, ss_num_2, ss_client_port_2, ss_ip_2);
                    insert_at_front(new_lru_node, head);
                }
                if(send(sock_ss2, bufferFor1, strlen(bufferFor1), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
            }
            char ack_ss2_full[BUF_SIZE];
            if(recv(sock_ss2, ack_ss2_full, sizeof(ack_ss2_full), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            //Send ack to SS1
            if(send(sock_ss1, ack_ss2_full, strlen(ack_ss2_full), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            //Send ack to the client
            if(send(nm_sock_for_client, ack_ss2_full, strlen(ack_ss2_full), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
        }
    }