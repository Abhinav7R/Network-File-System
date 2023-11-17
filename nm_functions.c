#include "headers.h"


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

int what_to_do(char* input)
{
    if(strncmp(input,"read",strlen("read"))==0)
    {
        char* filename = strtok(input," ");
        filename = strtok(NULL," ");
        printf("filename: %s\n",filename);

    }
    else if(strncmp(input,"write",strlen("write"))==0)
    {

    }
    else if(strncmp(input,"retrieve",strlen("retrieve"))==0)
    {

    }
    else if(strncmp(input,"create",strlen("create"))==0)
    {

    }
    else if(strncmp(input,"delete",strlen("delete"))==0)
    {

    }
    else if(strncmp(input,"copy",strlen("copy"))==0)
    {

    }
    else if(strncmp(input,"create folder",strlen("create folder"))==0)
    {

    }
    else if(strncmp(input,"delete folder",strlen("delete folder"))==0)
    {

    }
    else if(strncmp(input,"copy folder",strlen("copy folder"))==0)
    {

    }
    else
    {
        printf("Invalid action\n");
        return 0;
    }

}