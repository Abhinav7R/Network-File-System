#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <tries.c>

// Assumptions: There are 10 Storage Servers in the NFS

// Storage Server Sends:
// 1] The IP address of the Storage Server.
// 2] The port on which the Storage Server is listening for connections from the Naming Server.
// 3] The port on which the Storage Server is listening for connections from clients.
// 4] The file paths that clients can access on the Storage Server.

int main()
{
    int server_sock;
    int client_sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
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
    server_addr.sin_port = htons(1234);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket
    int n = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to port 1234\n");

    // Listen for incoming connections
    int n = listen(server_sock, 5);
    if (n < 0)
    {
        perror("[-]Listen error");
        exit(1);
    }
    printf("[+]Listening for Storage Servers...\n");

    // Intialize Tries
    trie *root = init();

    int count = 0;

    while (count < 10)
    {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);

        if (client_sock < 0)
        {
            perror("[-]Accept error");
            exit(1);
        }

        // Receive initialization details from Storage Server
        bzero(B, 1024);
        if (recv(client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }

        // Extract IP, Naming Server Port and Client Port from the received B
        char ip[21];
        int nm_port, client_port;
        sscanf(B, "IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\n", ip, &nm_port, &client_port);

        // Extract file paths and insert them into Tries
        while (1)
        {
            bzero(B, 1024);
            if (recv(client_sock, B, sizeof(B), 0) < 0)
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
        strcpy(B, "Connection successful");

        if (send(client_sock, B, sizeof(B), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }

        close(client_sock);
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