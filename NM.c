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

int what_to_do(char* input, int nm_sock_for_client);

int main()
{
    init_ss_info();
    // Intialize Tries
    root = init();
    // Initialize LRU Cache
    head = init_lru();

    int server_sock;
    int ss_as_client_sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in ss_as_client_addr;
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

    

    int count = 0;

    // Accept connections from Storage Servers
    //for now from 1 storage server
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

    printf("[+]All Storage Servers connected. Now accepting client connections.\n");

    // Print all paths stored in the Trie
    printf("[+]Paths stored in Trie:\n");
    print_all_strings_in_trie(root);


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

    //send connection successful to client
    addr_size_for_client = sizeof(nm_addr_for_client);
    int client_sock = accept(nm_sock_for_client, (struct sockaddr*)&nm_addr_for_client, &addr_size_for_client);
    if(client_sock < 0)
    {
        perror("[-]Accept error");
        exit(1);
    }

    printf("[+]Client connected\n");

    bzero(buf,BUF_SIZE);
    strcpy(buf,"Connection successful");
    if(send(client_sock,buf,strlen(buf),0)<0)
    {
        perror("send() error");
        exit(1);
    }

    char input[BUF_SIZE];
    if(recv(client_sock,input,BUF_SIZE,0)<0)
    {
        perror("recv() error");
        exit(1);
    }

    printf("Received from client: %s\n",buf);
    what_to_do(input,client_sock);
    
    // Close the Naming Server socket for storage server
    close(server_sock);

    return 0;
}