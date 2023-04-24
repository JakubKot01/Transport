#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

size_t send_packet(int sockfd, char message[20], struct sockaddr_in server_address) {
    return sendto(sockfd, message, strlen(message), 0, 
            (struct sockaddr*) &server_address, sizeof(server_address));
}

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

    server_address.sin_port = htons(atoi(argv[2]));
    if(server_address.sin_port == 0) {
        fprintf(stderr, "Invalid port number!\n");
        return -1;
    }

    int length = atoi(argv[4]);
    if(length == 0) {
        fprintf(stderr, "Invalid length of file!\n");
        return -1;
    }

    FILE *fd = fopen(argv[3], "w");
    if (!fd) {
        fprintf(stderr, "Failed to open a file\n");
        return -1;
    }

    char message[20];
    int start = 0;
    int received = 0;

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int to_send;

    fd_set descriptors;

    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    char buffer[IP_MAXPACKET];

    while(length) {
        if (length < 1000) to_send = length;
        else to_send = 1000;
        sprintf(message, "GET %d %d\n", start, to_send);
        if (send_packet(sockfd, message, server_address) != strlen(message)) {
            fprintf(stderr, "Failed to send a request: %s\n", strerror(errno));
            return -1;
        }
        FD_ZERO(&descriptors);
        FD_SET(sockfd, &descriptors);

        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);

        if(ready < 0) return -1;
        if(ready == 0) send_packet(sockfd, message, server_address);

        ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, 
                                (struct sockaddr*)&sender, &sender_len);

        fprintf(stderr, "Received\n");

        if(packet_len < 0) {
            fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
            return -1;
        }

        if(sender.sin_addr.s_addr == server_address.sin_addr.s_addr
            && sender.sin_port == server_address.sin_port) {
                char checker[10];
                strncpy(checker, buffer, 4);
                if (!strcmp(checker, "DATA")) {
                    int offset = packet_len - to_send;
                    fwrite(buffer + offset, sizeof(char), to_send, fd);
                    fprintf(stderr, "Writen\n");
                }
                received += to_send;
                length -= to_send;
            }
    }

    close(fd);
    return 0;
}