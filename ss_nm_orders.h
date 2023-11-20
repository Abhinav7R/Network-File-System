#include "headers.h"

void make_file(char* file, int nm_sockfd);
void del_file(char* file, int nm_sockfd);
void delete_dir(char* name, int nm_sockfd);
void make_dir(char* name, int nm_sockfd);
void copyFile(char* file, char* dir, int nm_sockfd);
void copyDir(char* dir, char* dest, int nm_sockfd);
void recvFileFromSS(char* file, char* dest, int ss_port);
// void sendFileToSS(char* file, char* dest, int port_num);
void recvDirFromSS(char* dir, char* dest, int ss_port);
// void sendDirToSS(char* dir, char* dest, int port_num);