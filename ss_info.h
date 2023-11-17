#ifndef __SS_INFO_H__
#define __SS_INFO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_STORAGE_SERVERS 100

typedef struct ss_info
{
    int ss_num;
    int ss_client_port;
    int ss_nm_port;
    char* ss_ip;
}ss_info;

#endif