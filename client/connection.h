#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#define h_addr h_addr_list[0]
#include "../common/common.h"
#include "client_conf.h"
#include "../common/common_conf.h"


int connect_to_server(char* hostname, int port);