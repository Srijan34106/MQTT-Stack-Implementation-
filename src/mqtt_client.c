#include "mqtt_client.h"
#include "mqtt_transport.h"
#include "mqtt_encode.h"
#include "mqtt_decode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MQTT_RX_BUFFER_SIZE 1024

// Internal structure definition
struct mqtt_client {
    mqtt_client_config_t cfg;
    int  sockfd;
    bool connected;
    uint16_t next_packet_id;
};

mqtt_client_t *mqtt_client_create(const mqtt_client_config_t *cfg) {
    if (!cfg || !cfg->host || cfg->port == 0) {
        fprintf(stderr, "mqtt_client_create: invalid configuration\n");
        return NULL;
    }

    mqtt_client_t *client = (mqtt_client_t *)calloc(1, sizeof(mqtt_client_t));
    if (!client) {
        perror("calloc");
        return NULL;
    }

    client->cfg = *cfg;
    client->sockfd = -1;
    client->connected = false;
    client->next_packet_id = 1;

    return client;
}

void mqtt_client_destroy(mqtt_client_t *client) {
    if (!client) return;

    if (client->connected)
        mqtt_client_disconnect(client);

    free(client);
}

static uint16_t mqtt_client_get_next_packet_id(mqtt_client_t *client) {
    uint16_t id = client->next_packet_id++;
    if (client->next_packet_id == 0) {
        client->next_packet_id = 1;
    }
    return id;
}

int mqtt_client_connect(mqtt_client_t *client) {
    if (!client) return -1;

    if (client->connected) {
        fprintf(stderr, "mqtt_client_connect: already connected\n");
        return 0;
    }

    printf("Connecting to %s:%u ...\n",
           client->cfg.host,
           (unsigned int)client->cfg.port);

    int sockfd = mqtt_transport_connect(client->cfg.host, client->cfg.port);
    if (sockfd < 0) {
        fprintf(stderr, "TCP connection failed.\n");
        return -1;
    }

    client->sockfd = sockfd;

    // --- MQTT CONNECT ---
    uint8_t packet[256];
    int len = mqtt_encode_connect(packet, sizeof(packet),
                                  client->cfg.client_id,
                                  client->cfg.keep_alive_sec);
    if (len < 0) {
        fprintf(stderr, "Failed to encode CONNECT packet\n");
        mqtt_transport_close(client->sockfd);
        client->sockfd = -1;
        return -1;
    }

    if (mqtt_transport_send(client->sockfd, packet, len) != len) {
        fprintf(stderr, "Error sending CONNECT packet\n");
        mqtt_transport_close(client->sockfd);
        client->sockfd = -1;
        return -1;
    }

    uint8_t resp[4];
    int r = mqtt_transport_recv(client->sockfd, resp, sizeof(resp));
    if (r <= 0) {
        fprintf(stderr, "Error receiving CONNACK\n");
        mqtt_transport_close(client->sockfd);
        client->sockfd = -1;
        return -1;
    }

    if (mqtt_decode_connack(resp, r) != 0) {
        fprintf(stderr, "Invalid CONNACK response\n");
        mqtt_transport_close(client->sockfd);
        client->sockfd = -1;
        return -1;
    }

    printf("CONNACK received → MQTT CONNECT success!\n");

    client->connected = true;
    return 0;
}

void mqtt_client_disconnect(mqtt_client_t *client) {
    if (!client) return;
    if (!client->connected) return;

    printf("Closing TCP connection.\n");
    mqtt_transport_close(client->sockfd);

    client->sockfd = -1;
    client->connected = false;
}

int mqtt_client_loop(mqtt_client_t *client) {
    if (!client || !client->connected) {
        fprintf(stderr, "mqtt_client_loop: not connected\n");
        return -1;
    }

    uint8_t buf[MQTT_RX_BUFFER_SIZE];

    int r = mqtt_transport_recv(client->sockfd, buf, sizeof(buf));
    if (r < 0) {
        fprintf(stderr, "Error receiving data\n");
        return -1;
    }
    if (r == 0) {
        fprintf(stderr, "Connection closed by broker\n");
        return -1;
    }

    uint8_t packet_type = buf[0] >> 4;

    if (packet_type == 3) { // PUBLISH
        char topic[256];
        const uint8_t *payload = NULL;
        size_t payload_len = 0;

        if (mqtt_decode_publish_qos0(buf, (size_t)r,
                                     topic, sizeof(topic),
                                     &payload, &payload_len) == 0) {
            printf("Incoming PUBLISH: topic='%s', payload_len=%zu\n",
                   topic, payload_len);
            if (client->cfg.on_message) {
                client->cfg.on_message(topic, payload, payload_len);
            }
        } else {
            fprintf(stderr, "Failed to decode PUBLISH packet\n");
        }
    } else if (packet_type == 13) { // PINGRESP
        printf("PINGRESP received (not used yet).\n");
    } else {
        printf("Received packet type %u (ignored in this simple client)\n",
               packet_type);
    }

    return 0;
}

int mqtt_client_publish_qos0(mqtt_client_t *client,
                             const char *topic,
                             const uint8_t *payload,
                             size_t payload_len) {
    if (!client || !client->connected) {
        fprintf(stderr, "mqtt_client_publish_qos0: not connected\n");
        return -1;
    }

    uint8_t packet[512];
    int len = mqtt_encode_publish_qos0(packet, sizeof(packet),
                                       topic, payload, payload_len);
    if (len < 0) {
        fprintf(stderr, "Failed to encode PUBLISH packet\n");
        return -1;
    }

    int sent = mqtt_transport_send(client->sockfd, packet, len);
    if (sent != len) {
        fprintf(stderr, "Failed to send full PUBLISH packet\n");
        return -1;
    }

    printf("PUBLISH sent to topic '%s', payload_len=%zu\n", topic, payload_len);
    return 0;
}

int mqtt_client_subscribe_qos0(mqtt_client_t *client,
                               const char *topic) {
    if (!client || !client->connected) {
        fprintf(stderr, "mqtt_client_subscribe_qos0: not connected\n");
        return -1;
    }

    uint8_t packet[256];
    uint16_t packet_id = mqtt_client_get_next_packet_id(client);

    int len = mqtt_encode_subscribe_qos0(packet, sizeof(packet),
                                         packet_id, topic);
    if (len < 0) {
        fprintf(stderr, "Failed to encode SUBSCRIBE packet\n");
        return -1;
    }

    if (mqtt_transport_send(client->sockfd, packet, len) != len) {
        fprintf(stderr, "Failed to send SUBSCRIBE packet\n");
        return -1;
    }

    // Wait for SUBACK (very simple, blocking)
    uint8_t resp[16];
    int r = mqtt_transport_recv(client->sockfd, resp, sizeof(resp));
    if (r <= 0) {
        fprintf(stderr, "Error receiving SUBACK\n");
        return -1;
    }

    if (mqtt_decode_suback(resp, (size_t)r) != 0) {
        fprintf(stderr, "SUBACK decode failed\n");
        return -1;
    }

    printf("SUBACK received → subscription to '%s' successful.\n", topic);
    return 0;
}

