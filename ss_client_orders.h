#include "headers.h"

void read_file(char* file, char* buffer_client, int client_sockfd);
void write_file(char* file, char* buffer_client, int client_sockfd);
void retrieve_info(char* file, char* buffer_client, int client_sockfd);