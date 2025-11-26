#ifndef MQTT_TRANSPORT_H
#define MQTT_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

/**
 * Connect to a TCP server.
 *
 * @param host  Hostname or IP string (e.g., "test.mosquitto.org")
 * @param port  TCP port (e.g., 1883)
 *
 * @return socket file descriptor on success, -1 on failure
 */
int mqtt_transport_connect(const char *host, uint16_t port);

/**
 * Send data over an open socket.
 *
 * @return number of bytes sent, or -1 on error.
 */
int mqtt_transport_send(int sockfd, const void *buf, size_t len);

/**
 * Receive data from an open socket.
 *
 * @return number of bytes received, 0 if connection closed, or -1 on error.
 */
int mqtt_transport_recv(int sockfd, void *buf, size_t maxlen);

/**
 * Close the socket.
 */
void mqtt_transport_close(int sockfd);

#endif // MQTT_TRANSPORT_H
