#include "mqtt_transport.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int mqtt_transport_connect(const char *host, uint16_t port) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sockfd = -1;
    char port_str[8];

    snprintf(port_str, sizeof(port_str), "%u", (unsigned int)port);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;   // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    int s = getaddrinfo(host, port_str, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1)
            continue;

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            // success
            break;
        }

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(result);

    if (sockfd == -1) {
        fprintf(stderr, "Failed to connect to %s:%u\n", host, (unsigned int)port);
    }

    return sockfd;
}

int mqtt_transport_send(int sockfd, const void *buf, size_t len) {
    ssize_t sent = send(sockfd, buf, len, 0);
    if (sent < 0) {
        perror("send");
        return -1;
    }
    return (int)sent;
}

int mqtt_transport_recv(int sockfd, void *buf, size_t maxlen) {
    ssize_t recvd = recv(sockfd, buf, maxlen, 0);
    if (recvd < 0) {
        perror("recv");
        return -1;
    }
    return (int)recvd; // can be 0 if connection closed
}

void mqtt_transport_close(int sockfd) {
    if (sockfd >= 0) {
        close(sockfd);
    }
}
