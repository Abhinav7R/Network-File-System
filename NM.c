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
char* nm_ip = "127.0.0.1";

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

        // Extract IP, Naming Server Port and Client Port from the received B
        char ip[21];
        int nm_port, client_port;
        sscanf(B, "IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\n", ip, &nm_port, &client_port);

        // send acknowledgement
        bzero(B, 1024);
        strcpy(B, "Connection successful");
        if (send(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }

        //change below code to 
        // Extract file paths and insert them into Tries
        while (1)
        {
            bzero(B, 1024);
            if (recv(ss_as_client_sock, B, sizeof(B), 0) < 0)
            {
                perror("[-]Receive error");
                exit(1);
            }

            if (strcmp(B, "DONE") == 0)
            {
                // All paths received
                break;
            }

            // Here count = Storage Server number
            insert(root, B, count);
        }
        count++;

        // Acknowledge connection
        bzero(B, 1024);
        strcpy(B, "Filepaths received");

        if (send(ss_as_client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }

        close(ss_as_client_sock);
    }

    printf("[+]All Storage Servers connected. Now accepting client connections.\n");

    // Print all paths stored in the Trie
    printf("[+]Paths stored in Trie:\n");
    print_all_strings_in_trie(root);


    // Accept CLient connection and process its queries
    
    // Close the Naming Server socket
    close(server_sock);

    return 0;
}