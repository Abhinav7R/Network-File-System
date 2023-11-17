#include "headers.h"

extern trie* root;
extern lru_head* head;
extern ss_info* array_of_ss_info;

    //receive action from client
    //for read: read filename
    //for write: write filename and then after receiving ack write data
    //for retrieve information: retrieve filename
    //for create: create file filepath
    //for delete: delete filename with path
    //for copy file: copy filepath to newfilepath
    //for create folder: create folder folderpath
    //for delete folder: delete folder folderpath
    //for copy folder: copy folder folderpath to newfolderpath

int what_to_do(char* input, int nm_sock_for_client)
{
    if(strncmp(input,"read",strlen("read"))==0)
    {
        char* filename = strtok(input," ");
        filename = strtok(NULL," ");
        printf("filename: %s\n",filename);
        lru_node* node_in_cache = find_and_return(filename,head);
        if(node_in_cache == NULL)
        {
            //find in trie
            int ss_num=search(root,filename);
            if(ss_num==0)
            {
                //send -1 as ack
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client,"%d",-1);
                if(send(nm_sock_for_client,send_details_to_client,strlen(send_details_to_client),0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 0;
            }
            else
            {
                char send_details_to_client[BUF_SIZE];
                int port=array_of_ss_info[ss_num-1].ss_client_port;
                char ss_ip[21];
                strcpy(ss_ip,array_of_ss_info[ss_num-1].ss_ip);
                //concatenate ip and port as a string separated by space
                sprintf(send_details_to_client,"%s %d",ss_ip,port);
                if(send(nm_sock_for_client,send_details_to_client,strlen(send_details_to_client),0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
            }
        }
        else
        {
            char send_details_to_client[BUF_SIZE];
            int port=node_in_cache->storage_server_port_for_client;
            char ss_ip[21];
            strcpy(ss_ip,node_in_cache->storage_server_ip);
            //concatenate ip and port as a string separated by space
            sprintf(send_details_to_client,"%s %d",ss_ip,port);
            if(send(nm_sock_for_client,send_details_to_client,strlen(send_details_to_client),0)<0)
            {
                perror("send() error");
                exit(1);
            }
        }

    }
    else if(strncmp(input,"write",strlen("write"))==0)
    {

    }
    else if(strncmp(input,"retrieve",strlen("retrieve"))==0)
    {

    }

    else if(strncmp(input,"create_file",strlen("create"))==0)
    {
        //if file already exists send ack as -1
        //choose ss number by comparing string by removing file name
        //make connection with storage server and send action
        //receive ack from storage server
        //add to trie the file and storage server number
        //add to lru cache
    }
    else if(strncmp(input,"delete_file",strlen("delete"))==0)
    {
        
    }
    else if(strncmp(input,"copy_file",strlen("copy"))==0)
    {

    }
    else if(strncmp(input,"create_folder",strlen("create folder"))==0)
    {

    }
    else if(strncmp(input,"delete_folder",strlen("delete folder"))==0)
    {

    }
    else if(strncmp(input,"copy_folder",strlen("copy folder"))==0)
    {

    }
    else
    {
        printf("Invalid action\n");
        return 0;
    }

}