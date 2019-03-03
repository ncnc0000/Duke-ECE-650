#include "potato.h"

using namespace std;

int main(int argc, char* argv[]){
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    const char *hostname = argv[1];
    const char *port     = argv[2];
    int sockfd_ringmaster, sockfd_bind, sockfd_connect, sockfd_accept;
    struct addrinfo hints,hints_connect, hints_as_server,*connect_list, *servinfo, *res,*p;
    int status, numbytes;
    int yes = 1;
    player_meg player_info;
    //player bind
    memset(&hints_as_server, 0, sizeof(hints_as_server));
    hints_as_server.ai_family = AF_UNSPEC;
    hints_as_server.ai_socktype = SOCK_STREAM;
    hints_as_server.ai_flags = AI_PASSIVE;
    int num;
    string temp_str;
    while(1){
        srand((unsigned int) time(NULL));
        num = rand() % 64000 + 1025;
        stringstream strs1;
        strs1 << num;
        temp_str = strs1.str();
        const char* port_player = temp_str.c_str();
        if((status = getaddrinfo(NULL, port_player, &hints_as_server, &res)) != 0){
            cerr << "Error: cannot get address info for host" << endl;
            cerr << "  (" << hostname << "," << port_player << ")" << endl;
            return -1;
        }
        if ((sockfd_bind = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
            fprintf(stderr, "client %d : socket \n", player_info.player_id);
        }
        if (setsockopt(sockfd_bind, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd_bind, res->ai_addr, res->ai_addrlen) == -1) {
            continue;
            close(sockfd_bind);
            perror("bind: \n");
            fprintf(stderr, "client %d : bind \n", player_info.player_id);
            return -1;
        }
        break;
    }
    
    //player connect to ringmaster
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_ringmaster = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd_ringmaster, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd_ringmaster);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    //listen to other connection
    if (listen(sockfd_bind,1) == -1) {
        perror("listen");
        exit(1);
    }
    //send bind port of each player
    if(send(sockfd_ringmaster, &num, sizeof(num), 0) == -1){
        perror("send");
    }
    //receive all player info
    if ((numbytes = recv(sockfd_ringmaster, &player_info, sizeof(player_info), 0)) == -1) {
        perror("recv");
        exit(1);
    }
    printf("Connected as player %d out of %d total players\n", player_info.player_id, player_info.num_player);
    
    //connect and accpet from nei
    stringstream strs;
    strs << player_info.right_nei_port;
    temp_str = strs.str();
    const char* right_nei_port = temp_str.c_str();
    memset(&hints_connect, 0, sizeof(hints_connect));
    hints_connect.ai_family   = AF_UNSPEC;
    hints_connect.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(player_info.right_nei_ip, right_nei_port, &hints_connect, &connect_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << right_nei_port << ")" << endl;
        return -1;
    }
    if ((sockfd_connect = socket(connect_list->ai_family, connect_list->ai_socktype, connect_list->ai_protocol)) == -1) {
        perror("client: socket for right nei\n");
    }
    if (connect(sockfd_connect, connect_list->ai_addr, connect_list->ai_addrlen) == -1) {
        close(sockfd_connect);
        perror("client: connect for right \n");
    }
    addr_size = sizeof(client_addr);
    sockfd_accept = accept(sockfd_bind, (struct sockaddr *)&client_addr, &addr_size);
    if (sockfd_accept == -1) {
        perror("accept");
    }
    //player send ready info to notify ringmaster
    send(sockfd_ringmaster, &player_info.player_id, sizeof(player_info.player_id), 0);
    fd_set readfds;
    potato mypotato;
    mypotato.flag = 0;
    int max = 0;
    int master_sock;
    while(1){
        FD_ZERO(&readfds);
        FD_SET(sockfd_connect, &readfds);
        if(sockfd_connect > max){
            max = sockfd_connect;
        }
        FD_SET(sockfd_accept, &readfds);
        if(sockfd_accept > max){
            max = sockfd_accept;
        }
        FD_SET(sockfd_ringmaster, &readfds);
        if(sockfd_ringmaster > max){
            max = sockfd_ringmaster;
        }
        if(select(max + 1,&readfds,NULL,NULL,NULL) > 0){
            if(FD_ISSET(sockfd_ringmaster,&readfds)>0){
                master_sock = sockfd_ringmaster;
            }
            else if(FD_ISSET(sockfd_accept,&readfds)>0){
                master_sock = sockfd_accept;
            }
            else if(FD_ISSET(sockfd_connect,&readfds)>0){
                master_sock = sockfd_connect;
            }
        }
        if((numbytes = recv(master_sock, &mypotato, sizeof(mypotato), 0)) == -1){
            perror("Potato Received ");
        }
        
        if(numbytes == 0 || mypotato.flag == 1){
            close(sockfd_accept);
            close(sockfd_connect);
            close(sockfd_ringmaster);
            close(sockfd_bind);
            break;
        }
        sleep(1);
        mypotato.trace[mypotato.entire_hops - mypotato.remain_hops] = player_info.player_id;
        mypotato.remain_hops--;
        if(mypotato.remain_hops == 0){
            if(send(sockfd_ringmaster, &mypotato, sizeof(mypotato), 0) == -1){
                perror("Potato Send back to ringmaster: ");
            }
            printf("I’m it\n");
        }
        else{
            srand(time(NULL) + player_info.player_id);
            num = (rand()) % 500;
            //send potato to left nei
            if(num < 250){
                if(send(sockfd_accept, &mypotato, sizeof(mypotato), 0) == -1){
                    perror("Potato Send to left nei: ");
                }
                printf("Sending potato to %d\n", player_info.left_nei);
            }
            //send potato to right nei
            else{
                if(send(sockfd_connect, &mypotato, sizeof(mypotato), 0) == -1){
                    perror("Potato Send to left nei: ");
                }
                printf("Sending potato to %d\n", player_info.right_nei);
            }
        }
    }
    freeaddrinfo(servinfo);
    freeaddrinfo(connect_list);
    freeaddrinfo(res);
    return 0;
}