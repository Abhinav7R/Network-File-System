#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "tries.h"
#include "lru.h"
#include "ss_info.h"
#include "ss_nm_orders.h"
#include "ss_client_orders.h"

#define BUF_SIZE 1024

#endif