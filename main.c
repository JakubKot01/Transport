#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

int main(int argc, char* argv[]){
    if (argc != 5) {
        fprintf(stderr, "Invalid arguments!\n");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;

    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP address!\n");
        return -1;
    }

    server_address.sin_port = htons((int)argv[2]);
    if(server_address.sin_port == 0) {
        fprintf(stderr, "Invalid port number!\n");
        return -1;
    }

    int length = (int)argv[4];
    if(length == 0) {
        fprintf(stderr, "Invalid length of file!\n");
        return -1;
    } 

    FILE *fd = open(argv[3], "w");
    if (!fd) {
        fprintf(stderr, "Failed to open a file\n");
        return -1;
    }

    char message[20];
    int start = 0;
    int to_send;
    while(length) {
        if (length < 1000) to_send = length;
        else to_send = 1000;
        sprintf(message, "GET %ld %ld\n", start, to_send);
        if (sendto(sockfd, message, strlen(message), 0, 
            (struct sockaddr*) &server_address, sizeof(server_address)) != strlen(message)) {
            fprintf(stderr, "Failed to send a request: %s\n", strerror(errno));
            return -1;
        }

    }

    close(fd);
    return 0;
}