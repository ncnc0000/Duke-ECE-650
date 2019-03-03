#ifndef __MY_MALLOC__
#define __MY_MALLOC__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <sstream>
#include <string.h>
#include <time.h>
#include <cstring>

typedef struct messages_t{
    int player_id;
    int player_port;
    int num_player;
    int right_nei;
    int right_nei_port;
    // const char * right_nei_ip;
    char right_nei_ip[INET6_ADDRSTRLEN];
    int left_nei;
    int left_nei_port;
    //const char * left_nei_ip;
    char left_nei_ip[INET6_ADDRSTRLEN];
}messages_t;

typedef struct messages_t player_meg;


typedef struct potato_t{
    int entire_hops;
    int remain_hops;
    int flag;
    int trace[512];
}potato_t;

typedef struct potato_t potato;




#endif