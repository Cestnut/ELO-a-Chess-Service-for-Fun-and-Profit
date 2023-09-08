#include "connection.h"

int connect_to_server(char* hostname, int port){
    int socket_fd;
    struct sockaddr_in server_addr;
    char input_buffer[BUFFER_LEN], output_buffer[BUFFER_LEN];
    
    memset(input_buffer, 0, BUFFER_LEN);
    memset(output_buffer, 0, BUFFER_LEN);


    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1){
        printf("Error opening socket: errno %d\n", errno);
        exit(0);
    }
    printf("Socket created\n");


    struct hostent *server;
    server = gethostbyname(hostname);
    if(server == NULL){
        printf("Error resolving hostname\n");
        exit(0);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)server->h_addr);


    if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("Error connecting: errno %d\n", errno);
        exit(0);
    }
    printf("Connected succesfully\n");

    return socket_fd;
}