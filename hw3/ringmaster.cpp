#include "potato.h"

using namespace std;

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

uint16_t get_in_port(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return ((struct sockaddr_in*)sa)->sin_port;
    }
    return ((struct sockaddr_in6*)sa)->sin6_port;
}

int main(int argc, char *argv[]){
    if(argc < 4){
        printf("ringmaster: arguments!\n");
        return -1;
    }
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res, *p;
    vector<int> client_fd, client_port;
    player_meg player_messages;
    int sockfd, new_fd;
    int status;
    int yes = 1;
    const char * MYPORT = argv[1];
    int hops = atoi(argv[3]);
    int BACKLOG =atoi(argv[2]);
    char **client_ip = NULL;
    char s[INET6_ADDRSTRLEN];
    const char *hostname = NULL;
    if(BACKLOG <= 1){
        printf("ringmaster: player!\n");
        return -1;
    }
    if(hops < 0 || hops > 512){
        printf("ringmaster: hops!\n");
        return -1;
    }
    printf("Potato Ringmaster\n");
    printf("Players = %d\n", BACKLOG);
    printf("Hops = %d\n", hops);
    memset(&player_messages, 0, sizeof(player_messages));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(NULL, MYPORT, &hints, &res)) != 0){
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << MYPORT << ")" << endl;
        return -1;
    }

    for(p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    int i = 0;
    int port, numberbyte;
    while(i != BACKLOG){
        addr_size = sizeof(client_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        client_fd.push_back (new_fd);
        inet_ntop(client_addr.ss_family,get_in_addr((struct sockaddr *)&client_addr),s,sizeof(s));
        client_ip = (char**)realloc(client_ip, (i + 1) * sizeof(*client_ip));
        client_ip[i] = s;
        if((numberbyte = recv(new_fd, &port, sizeof(port), 0)) == -1){
            perror("recv");
        }
        client_port.push_back(port);
        i++;
    }
    for(int i = 0; i < BACKLOG; i++){
        if(i == 0){
            player_messages.player_id = 0;
            player_messages.player_port = client_port[0];
            player_messages.num_player = BACKLOG;
            player_messages.left_nei = BACKLOG - 1;
            player_messages.left_nei_port = client_port[BACKLOG - 1];
            strcpy(player_messages.left_nei_ip,client_ip[BACKLOG - 1]);
            player_messages.right_nei = 1;
            player_messages.right_nei_port = client_port[1];
            strcpy(player_messages.right_nei_ip,client_ip[1]);
        }
        else if(i == BACKLOG - 1){
            player_messages.player_id = BACKLOG - 1;
            player_messages.player_port = client_port[BACKLOG - 1];
            player_messages.num_player = BACKLOG;
            player_messages.left_nei = BACKLOG - 2;
            player_messages.left_nei_port = client_port[BACKLOG - 2];
            strcpy(player_messages.left_nei_ip, client_ip[BACKLOG - 2]);
            player_messages.right_nei = 0;
            player_messages.right_nei_port = client_port[0];
            strcpy(player_messages.right_nei_ip, client_ip[0]);
        }
        else{
            player_messages.player_id = i;
            player_messages.player_port = client_port[i];
            player_messages.num_player = BACKLOG;
            player_messages.left_nei = i - 1;
            player_messages.left_nei_port = client_port[i - 1];
            strcpy(player_messages.left_nei_ip, client_ip[i - 1]);
            player_messages.right_nei = i + 1;
            player_messages.right_nei_port = client_port[i + 1];
            strcpy(player_messages.right_nei_ip,client_ip[i + 1]);
        }
        if(send(client_fd[i], &player_messages, sizeof(player_messages),0) == -1){
            perror("send");
        }
    }
    int ready = 0;
    int id;
    while(ready != BACKLOG){
        for(vector<int>::iterator it = client_fd.begin(); it != client_fd.end(); ++it){
            if((numberbyte = recv(*it, &id, sizeof(id), 0)) == -1){
                perror("Ready: ");
            }
            printf("Player %d is ready to play\n", id);
            ready++;
        }
    }
    potato mypotato;
    if(hops != 0){
        fd_set readfds;
        int num;
        mypotato.entire_hops = hops;
        mypotato.remain_hops = hops;
        mypotato.flag = 0;
        for(int i = 0; i < 512; i++){
            mypotato.trace[i] = 0;
        }
        srand((unsigned int) time(NULL));
        num = rand() % BACKLOG;
        if(send(client_fd[num], &mypotato, sizeof(mypotato), 0) == -1){
            perror("Potato Send ");
        }
        printf("Ready to start the game, sending potato to player %d\n", num);
        int max = 0;
        while(1){
            FD_ZERO(&readfds);
            for(vector<int>::iterator it = client_fd.begin(); it != client_fd.end(); ++it){
                FD_SET(*it, &readfds);
                if(*it > max){
                    max = *it;
                }
            }
            if(select(max + 1,&readfds,NULL,NULL,NULL) > 0){
                for(vector<int>::iterator it = client_fd.begin(); it != client_fd.end(); ++it){
                    if(FD_ISSET(*it,&readfds)>0){
                        if((numberbyte = recv(*it, &mypotato, sizeof(mypotato), 0)) == -1){
                            perror("Potato Receive");
                        }
                        if(mypotato.remain_hops == 0){
                            printf("Trace of potato:\n");
                            for(int i = 0; i < hops; i++){
                                if( i == hops - 1){
                                    printf("%d \n", mypotato.trace[i]);
                                }
                                else{
                                    printf("%d,", mypotato.trace[i]);
                                }
                            }
                            break;
                        }
                        else{
                            perror("BAD GAME");
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    //shutdown game
    mypotato.flag = 1;
    for(int i = 0; i < BACKLOG; i++){
        if(send(client_fd[i], &mypotato, sizeof(mypotato),0) == -1){
            perror("send");
        }
    }
    freeaddrinfo(res);
    free(client_ip);
    for(vector<int>::iterator it = client_fd.begin(); it != client_fd.end(); ++ it){
        close(*it);
    }
    return 0;
}