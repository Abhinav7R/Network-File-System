#include "headers.h"

// Assumptions: There are 10 Storage Servers in the NFS

// Storage Server Sends:
// 1] The IP address of the Storage Server.
// 2] The port on which the Storage Server is listening for connections from the Naming Server.
// 3] The port on which the Storage Server is listening for connections from clients.
// 4] The file paths that clients can access on the Storage Server.

extern trie* root;
extern lru_head* head;
extern ss_info* array_of_ss_info;

#define naming_server_port 4545
#define nm_port_for_clients 4546
char* nm_ip = "127.0.0.1";

int count = 0;
pthread_mutex_t count_lock=PTHREAD_MUTEX_INITIALIZER;

//3 semaphores for critical sections of LRU tries and ss_info
sem_t lru_lock;
sem_t trie_lock;
sem_t ss_info_lock;

//initialise semaphores
void init_semaphores()
{
    sem_init(&lru_lock,0,1);
    sem_init(&trie_lock,0,1);
    sem_init(&ss_info_lock,0,1);
}

int what_to_do(char* input, int nm_sock_for_client);

typedef struct arguments_for_ss_thread
{
    int server_sock;
    struct sockaddr_in server_addr;
}arguments_for_ss_thread;

typedef struct arguments_for_client_thread
{
    int client_sock;
    struct sockaddr_in client_addr;
}arguments_for_client_thread;

typedef struct arguments_for_individual_client_thread
{
    int client_id;
    int client_sock;
    struct sockaddr_in client_addr;
}arguments_for_individual_client_thread;

ss_backups ss_ke_backups[NUM_STORAGE_SERVERS];
int available_ss[NUM_STORAGE_SERVERS];

void backup_for_3_ss()
{
    //back up for 3 ss
    //ss1
    ss_ke_backups[1].backup_ss1=2;
    ss_ke_backups[1].backup_ss2=3;
    //agrim
    //create a folder called ss1_backup in ss2 and ss3
    //copy folder ss1 to ./ss2/ss1_backup and ./ss3/ss1_backup
    //ss2
    ss_ke_backups[2].backup_ss1=1;
    ss_ke_backups[2].backup_ss2=3;
    //agrim
    //ss3
    ss_ke_backups[3].backup_ss1=1;
    ss_ke_backups[3].backup_ss2=2;
    //agrim
}

void* backup_for_more_than_3(int ss_num)
{
    for(int i=ss_num;i>0;i--)
    {
        if(available_ss[i]==AVAILABLE)
        {
            ss_ke_backups[ss_num].backup_ss1=i;
            break;
        }
    }
    for(int i=ss_num;i>0;i--)
    {
        if(available_ss[i]==AVAILABLE && i!=ss_ke_backups[ss_num].backup_ss1)
        {
            ss_ke_backups[ss_num].backup_ss2=i;
            break;
        }
    }
    //agrim
    //create a folder called ss1_backup in ss2 and ss3
    //copy folder ss1 to ./ss2/ss1_backup and ./ss3/ss1_backup
}

void* Handle_SS(void* arguments)
{
    arguments_for_ss_thread* args = (arguments_for_ss_thread*)arguments;
    // Accept connections from Storage Servers
    int server_sock=args->server_sock;
    struct sockaddr_in server_addr=args->server_addr;
    int ss_as_client_sock;
    struct sockaddr_in ss_as_client_addr;
    socklen_t addr_size;
    char B[1024];

    //initialise ss_ke_backups and available_ss
    for(int i=0;i<NUM_STORAGE_SERVERS;i++)
    {
        ss_ke_backups[i].backup_ss1=-1;
        ss_ke_backups[i].backup_ss2=-1;
        available_ss[i]=NOT_AVAILABLE;
    }
    
    while (count < NUM_STORAGE_SERVERS)
    {
        pthread_mutex_lock(&count_lock);
        count++;
        pthread_mutex_unlock(&count_lock);

        available_ss[count]=AVAILABLE;

        if(count==3)
        {
            backup_for_3_ss();
        }

        //assume that atleast 2 servers would be available for backup at any time
        else if(count>3)
        {
            backup_for_more_than_3(count);
        }

        addr_size = sizeof(ss_as_client_addr);
        ss_as_client_sock = accept(server_sock, (struct sockaddr *)&ss_as_client_addr, &addr_size);

        if (ss_as_client_sock < 0)
        {
            perror("[-]Accept error");
            exit(1);
        }

        // Receive initialization details from Storage Server
        bzero(B, 1024);
        if (recv(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        printf("[+]Received from Storage Server:\n%s\n", B);

        // Extract IP, Naming Server Port and Client Port from the received B
        char ip[21];
        int nm_port, client_port;
        sscanf(B, "IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\n", ip, &nm_port, &client_port);

        // send acknowledgement
        bzero(B, 1024);
        strcpy(B, "1st set of details received");
        if (send(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        //insert data in array_of_ss_info
        printf("count %d\n",count);
        printf("nmport: %d, clientport: %d\n",nm_port,client_port);
        insert_ss_info(count, client_port, nm_port, ip);

        // Extract file paths and insert them into Tries
        // tokenise B on | and insert each token into the Trie
        bzero(B, 1024);
        if (recv(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        char* token = strtok(B, "|");
        while(token != NULL)
        {
            insert(root, token, count);
            token = strtok(NULL, "|");
        }


        // Acknowledge connection
        bzero(B, 1024);
        strcpy(B, "Filepaths received");

        if (send(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }

        close(ss_as_client_sock);
        
        //reuse the socket
        if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(1);
        }
    }   

}

void* Handle_Client(void* argument_for_client)
{
    while(1)
    {
        arguments_for_individual_client_thread* args=(arguments_for_individual_client_thread*)argument_for_client;
        int client_id=args->client_id;
        int nm_sock_for_client=args->client_sock;
        struct sockaddr_in nm_addr_for_client=args->client_addr;
        socklen_t addr_size_for_client;
        char buf[BUF_SIZE];

        //send connection successful to client
        addr_size_for_client = sizeof(nm_addr_for_client);
        int client_sock = accept(nm_sock_for_client, (struct sockaddr*)&nm_addr_for_client, &addr_size_for_client);
        if(client_sock < 0)
        {
            perror("[-]Accept error");
            exit(1);
        }

        printf("[+]Client %d connected\n",client_id);

        bzero(buf,BUF_SIZE);
        strcpy(buf,"Connection successful");
        if(send(client_sock,buf,strlen(buf),0)<0)
        {
            perror("send() error");
            exit(1);
        }

        while(1)
        {
            char input[BUF_SIZE];
            bzero(input,BUF_SIZE);
            if(recv(client_sock,input,BUF_SIZE,0)<0)
            {
                perror("recv() error");
                exit(1);
            }

            // printf("Received from client: %s\n",buf);
            int check_exit=what_to_do(input,client_sock);
            if(check_exit==0)
                break;
        }
        printf("Client %d disconnected\n",client_id);
    }
}

void* Main_Handle_Client(void* argument_for_client)
{
    arguments_for_client_thread* args=(arguments_for_client_thread*)argument_for_client;
    int nm_sock_for_client=args->client_sock;
    struct sockaddr_in nm_addr_for_client=args->client_addr;
    socklen_t addr_size_for_client;

    //make NUM_CLIENTS client threads
    pthread_t client_thread_id[NUM_CLIENTS];
    //array of structs for arguments for client threads
    arguments_for_individual_client_thread* arguments_for_each_clients=(arguments_for_individual_client_thread*)malloc(sizeof(arguments_for_individual_client_thread)*NUM_CLIENTS);
    for(int i=0;i<NUM_CLIENTS;i++)
    {
        arguments_for_each_clients[i].client_id=i;
        arguments_for_each_clients[i].client_sock=nm_sock_for_client;
        arguments_for_each_clients[i].client_addr=nm_addr_for_client;
        pthread_create(&client_thread_id[i], NULL, Handle_Client, (void*)&arguments_for_each_clients[i]); 
    }

    for(int i=0;i<NUM_CLIENTS;i++)
    {
        pthread_join(client_thread_id[i], NULL);
    }

}

int main()
{
    init_ss_info();
    // Intialize Tries
    root = init();
    // Initialize LRU Cache
    head = init_lru();

    //initialise locks
    init_semaphores();

    int server_sock;
    // int ss_as_client_sock;
    struct sockaddr_in server_addr;
    // struct sockaddr_in ss_as_client_addr;
    socklen_t addr_size;
    char B[1024];

    // socket for the connection
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]Naming Server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // Replace with the desired port
    server_addr.sin_port = htons(naming_server_port);
    server_addr.sin_addr.s_addr = inet_addr(nm_ip);

    // Bind the server socket
    int n = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to port \n");

    // Listen for incoming connections
    int m = listen(server_sock, NUM_STORAGE_SERVERS);
    if (m < 0)
    {
        perror("[-]Listen error");
        exit(1);
    }
    printf("[+]Listening for Storage Servers...\n");


    // Create thread to handle Storage Server connections
    pthread_t ss_thread_id;
    arguments_for_ss_thread* arguments=(arguments_for_ss_thread*)malloc(sizeof(arguments_for_ss_thread));
    arguments->server_sock = server_sock;
    arguments->server_addr = server_addr;
    pthread_create(&ss_thread_id, NULL, Handle_SS, (void*)arguments);

    // Accept connections from Storage Servers
    //for now from 1 storage server
    /*
    while (count < 1)
    {
        count++;

        addr_size = sizeof(ss_as_client_addr);
        ss_as_client_sock = accept(server_sock, (struct sockaddr *)&ss_as_client_addr, &addr_size);

        if (ss_as_client_sock < 0)
        {
            perror("[-]Accept error");
            exit(1);
        }

        // Receive initialization details from Storage Server
        bzero(B, 1024);
        if (recv(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        printf("[+]Received from Storage Server:\n%s\n", B);

        // Extract IP, Naming Server Port and Client Port from the received B
        char ip[21];
        int nm_port, client_port;
        sscanf(B, "IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\n", ip, &nm_port, &client_port);

        // send acknowledgement
        bzero(B, 1024);
        strcpy(B, "1st set of details received");
        if (send(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        //insert data in array_of_ss_info
        printf("count %d\n",count);
        printf("nmport: %d, clientport: %d\n",nm_port,client_port);
        insert_ss_info(count, client_port, nm_port, ip);

        // Extract file paths and insert them into Tries
        // tokenise B on | and insert each token into the Trie
        bzero(B, 1024);
        if (recv(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        char* token = strtok(B, "|");
        while(token != NULL)
        {
            insert(root, token, count);
            token = strtok(NULL, "|");
        }


        // Acknowledge connection
        bzero(B, 1024);
        strcpy(B, "Filepaths received");

        if (send(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }

        close(ss_as_client_sock);
        //reuse the socket
        if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(1);
        }
    }
    */

    // printf("[+]All Storage Servers connected. Now accepting client connections.\n");

    // // Print all paths stored in the Trie
    // printf("[+]Paths stored in Trie:\n");
    // print_all_strings_in_trie(root);

    


    // Accept CLient connection and process its queries
    int nm_sock_for_client;
    struct sockaddr_in nm_addr_for_client;
    socklen_t addr_size_for_client;
    char buf[BUF_SIZE];
    
    // Create socket
    nm_sock_for_client = socket(PF_INET, SOCK_STREAM, 0);
    if (nm_sock_for_client < 0)
    {
        perror("socket() error");
        exit(1);
    }

    printf("[+]Naming Server socket for client created.\n");

    memset(&nm_addr_for_client, '\0', sizeof(nm_addr_for_client)); //init
    nm_addr_for_client.sin_family = AF_INET;
    nm_addr_for_client.sin_port = htons(nm_port_for_clients);
    nm_addr_for_client.sin_addr.s_addr = inet_addr(nm_ip);

    int nn = bind(nm_sock_for_client, (struct sockaddr*)&nm_addr_for_client, sizeof(nm_addr_for_client));
    if(nn < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }

    printf("[+]Bind to port for clients \n");

    int mm = listen(nm_sock_for_client, NUM_CLIENTS);
    if(mm < 0)
    {
        perror("[-]Listen error client");
        exit(1);
    }

    printf("[+]Listening for clients...\n");

    pthread_t client_thread_id;
    arguments_for_client_thread* arguments_for_clients=(arguments_for_client_thread*)malloc(sizeof(arguments_for_client_thread));
    arguments_for_clients->client_sock = nm_sock_for_client;
    arguments_for_clients->client_addr = nm_addr_for_client;
    pthread_create(&client_thread_id, NULL, Main_Handle_Client, (void*)arguments_for_clients);

    // //send connection successful to client
    // addr_size_for_client = sizeof(nm_addr_for_client);
    // int client_sock = accept(nm_sock_for_client, (struct sockaddr*)&nm_addr_for_client, &addr_size_for_client);
    // if(client_sock < 0)
    // {
    //     perror("[-]Accept error");
    //     exit(1);
    // }

    // printf("[+]Client connected\n");

    // bzero(buf,BUF_SIZE);
    // strcpy(buf,"Connection successful");
    // if(send(client_sock,buf,strlen(buf),0)<0)
    // {
    //     perror("send() error");
    //     exit(1);
    // }

    // while(1)
    // {
    //     char input[BUF_SIZE];
    //     bzero(input,BUF_SIZE);
    //     if(recv(client_sock,input,BUF_SIZE,0)<0)
    //     {
    //         perror("recv() error");
    //         exit(1);
    //     }

    //     // printf("Received from client: %s\n",buf);
    //     what_to_do(input,client_sock);
    // }

    pthread_join(ss_thread_id, NULL);
    pthread_join(client_thread_id, NULL);
    
    // Close the Naming Server socket for storage server
    close(server_sock);
    // Close the Naming Server socket for client
    close(nm_sock_for_client);



    return 0;
}