#include "headers.h"
#include "nm.h"

extern trie *root;
extern lru_head *head;
extern ss_info *array_of_ss_info;

extern ss_backups ss_ke_backups[NUM_STORAGE_SERVERS];
extern int available_ss[NUM_STORAGE_SERVERS];

int read_or_retrieve_file(char *input, int nm_sock_for_client)
{
    char *filename = strtok(input, " ");
        filename = strtok(NULL, " ");
        // printf("filename: %s\n", filename);
        lru_node *node_in_cache = find_and_return(filename, head);
        if (node_in_cache == NULL)
        {
            // printf("not in cache\n");
            // find in trie
            int ss_num = search(root, filename);
            rwlock_t *rwlock;
            rwlock = find_rwlock(root, filename);
            // printf("ss_num: %d\n", ss_num);
            if (ss_num == 0)
            {
                printf("file not found!!!\n");
                // send -1 as acknm_sock_for_client
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }
            else
            {
                // printf("found in trie\n");
                char send_details_to_client[BUF_SIZE];
                int port = array_of_ss_info[ss_num].ss_client_port;
                strcpy(send_details_to_client, array_of_ss_info[ss_num].ss_ip);

                // concatenate ip and port as a string separated by space
                char port_as_string[100];
                sprintf(port_as_string, " %d", port);
                strcat(send_details_to_client, port_as_string);
                
                acquire_readlock(rwlock);
                printf("read lock acquired\n");
                printf("ss details sent to client: %s#\n", send_details_to_client);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                //receive ack from client
                char ack[BUF_SIZE];
                if (recv(nm_sock_for_client, ack, sizeof(ack), 0) < 0)
                {
                    perror("recv() error");
                    exit(1);
                }
                release_readlock(rwlock);
                printf("read lock released\n");
                return 1;
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
            rwlock_t *rwlock;
            rwlock = find_rwlock(root, filename);
            acquire_readlock(rwlock);
            printf("read lock acquired\n");
            printf("ss details sent to client: %s#\n", send_details_to_client);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            //receive ack from client
            char ack[BUF_SIZE];
            if (recv(nm_sock_for_client, ack, sizeof(ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }
            release_readlock(rwlock);
            printf("read lock released\n");
        }
}

typedef struct backup_file_struct
{
    int ss_num;
    char* input;
}backup_file_struct;

void* main_backup_file(void* arg)
{
    backup_file_struct* bb = (backup_file_struct*)arg;
    int ss_num = bb->ss_num;
    char* input = bb->input;
    backup_create_file(ss_num,ss_ke_backups[ss_num].backup_ss1,input);
    backup_create_file(ss_num,ss_ke_backups[ss_num].backup_ss2,input);
}

void backup_create_file(int ss_num,int backup_num,char* input)
{
    int ss_nm_port = array_of_ss_info[backup_num].ss_nm_port;
    char *ss_ip = array_of_ss_info[backup_num].ss_ip;

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

    printf("Connected to Storage Server %d\n", backup_num);

    // manipulate input 
    // you have create_file ./ss(ss_num)/A/b.txt
    // you need to make it as ./ss(backup_num)/ss(ss_num)_backup/A/b.txt
    //eg: ./ss2/A/b.txt
    // will become ./ss3/ss2_backup/A/b.txt

    char input2[1024];


    // Send input to Storage Server
    if (send(sock2, input2, strlen(input2), 0) < 0)
    {
        perror("send() error");
        exit(1);
    }

    // Receive Ack from Storage Server
    int ack;
    char buffer_ack[BUF_SIZE];
    if (recv(sock2, buffer_ack, sizeof(buffer_ack), 0) < 0)
    {
        perror("recv() error");
        exit(1);
    }

    // Convert received acknowledgment to an integer
    sscanf(buffer_ack, "%d", &ack);

    close(sock2);
}


void do_backup_file(int ss_num, char* input)
{
    pthread_t create_file_thread;
    backup_file_struct bb;
    bb.ss_num = ss_num;
    bb.input = input;
    pthread_create(&create_file_thread,NULL,main_backup_file,(void*)&bb);
}