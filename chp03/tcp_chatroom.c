#include "chap03.h"

int main() {
    printf("Configuring local address...");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo * bind_addr;
    if(getaddrinfo(0, "8080", &hints, &bind_addr)) {
        fprintf(stderr, "getaddrinfo() failed %d", errno);
        return 1;
    }

    printf("Local address is: ");
    char addr_buffer[100];
    getnameinfo(bind_addr->ai_addr, bind_addr->ai_addrlen, addr_buffer, sizeof(addr_buffer), 0, 0, NI_NUMERICHOST);
    printf("%s\n", addr_buffer);

    printf("Creating socket...\n");
    int socket_listen;
    socket_listen = socket(bind_addr->ai_family, bind_addr->ai_socktype, bind_addr->ai_protocol);
    if(socket_listen < 0) {
        fprintf(stderr, "socket() failed %d", errno);
        return 1;
    }

    printf("Binding socket...\n");
    if(bind(socket_listen, bind_addr->ai_addr, bind_addr->ai_addrlen)) {
        fprintf(stderr, "bind() failed %d", errno);
        return 1;
    }
    freeaddrinfo(bind_addr);

    if(listen(socket_listen, 10) < 0){
        fprintf(stderr, "listen() failed %d", errno);
        return 1;
    }

    printf("Listening for connections...\n");

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    int max_socket = socket_listen;

    while (1) {
        fd_set reads;
        reads = master;
        if(select(max_socket+1, &reads, 0, 0, 0)) {
            fprintf(stderr, "select() failed %d", errno);
            return 1;
        }

        for(int i=0; i<max_socket; i++) {
            if(FD_ISSET(i, &reads)) {
                if(i == socket_listen) {
                    struct sockaddr_storage client_storage;
                    socklen_t client_len = client_len;
                    int socket_client;
                    socket_client = accept(socket_listen, (struct sockaddr *) &client_storage, &client_len);

                    FD_SET(socket_client, &master);

                    if(socket_client > max_socket) {
                        max_socket = socket_client;
                    }

                } else {
                    char buffer[1024];
                    int bytes_received = recv(i, buffer, sizeof(buffer), 0);

                    if(bytes_received < 0) {
                        FD_CLR(i, &master);
                        close(i);
                        continue;
                    }

                    int j;
                    for(j=1; j<=max_socket; j++) {
                        if(FD_ISSET(j, &master)) {
                            if(j == socket_listen || j == i) {
                                continue;
                            } else {
                                send(j, buffer, bytes_received, 0);
                            }
                        }
                    }
                }
            } 
        }
    }
    close(socket_listen);
    return 0;
}