#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mqtt_client.h"

static void print_message_callback(const char *topic,
                                   const uint8_t *payload,
                                   size_t payload_len) {
    printf(">>> Message received on topic '%s': ", topic);
    for (size_t i = 0; i < payload_len; ++i) {
        putchar((char)payload[i]);
    }
    putchar('\n');
    fflush(stdout);
}

static int run_connect_test(const char *host, uint16_t port) {
    mqtt_client_config_t cfg = {
        .host           = host,
        .port           = port,
        .client_id      = "srijan-mqtt-client",
        .keep_alive_sec = 30,
        .username       = NULL,
        .password       = NULL,
        .on_message     = NULL
    };

    printf("MQTT CLI (connect test). Host=%s, Port=%u\n",
           host, (unsigned int)port);

    mqtt_client_t *client = mqtt_client_create(&cfg);
    if (!client) {
        fprintf(stderr, "Failed to create MQTT client.\n");
        return 1;
    }

    if (mqtt_client_connect(client) != 0) {
        fprintf(stderr, "Failed to connect to broker.\n");
        mqtt_client_destroy(client);
        return 1;
    }

    mqtt_client_loop(client);

    mqtt_client_disconnect(client);
    mqtt_client_destroy(client);

    printf("MQTT connect test finished.\n");
    return 0;
}

static int run_publish(const char *host, uint16_t port,
                       const char *topic, const char *message) {
    mqtt_client_config_t cfg = {
        .host           = host,
        .port           = port,
        .client_id      = "srijan-mqtt-client",
        .keep_alive_sec = 30,
        .username       = NULL,
        .password       = NULL,
        .on_message     = NULL
    };

    printf("MQTT CLI (publish). Host=%s, Port=%u, Topic=%s, Message=%s\n",
           host, (unsigned int)port, topic, message);

    mqtt_client_t *client = mqtt_client_create(&cfg);
    if (!client) {
        fprintf(stderr, "Failed to create MQTT client.\n");
        return 1;
    }

    if (mqtt_client_connect(client) != 0) {
        fprintf(stderr, "Failed to connect to broker.\n");
        mqtt_client_destroy(client);
        return 1;
    }

    if (mqtt_client_publish_qos0(client, topic,
                                 (const uint8_t *)message,
                                 strlen(message)) != 0) {
        fprintf(stderr, "Failed to publish message.\n");
        mqtt_client_disconnect(client);
        mqtt_client_destroy(client);
        return 1;
    }

    mqtt_client_disconnect(client);
    mqtt_client_destroy(client);

    printf("Publish finished.\n");
    return 0;
}

static int run_subscribe(const char *host, uint16_t port,
                         const char *topic) {
    mqtt_client_config_t cfg = {
        .host           = host,
        .port           = port,
        .client_id      = "srijan-mqtt-client-sub",
        .keep_alive_sec = 30,
        .username       = NULL,
        .password       = NULL,
        .on_message     = print_message_callback
    };

    printf("MQTT CLI (subscribe). Host=%s, Port=%u, Topic=%s\n",
           host, (unsigned int)port, topic);

    mqtt_client_t *client = mqtt_client_create(&cfg);
    if (!client) {
        fprintf(stderr, "Failed to create MQTT client.\n");
        return 1;
    }

    if (mqtt_client_connect(client) != 0) {
        fprintf(stderr, "Failed to connect to broker.\n");
        mqtt_client_destroy(client);
        return 1;
    }

    if (mqtt_client_subscribe_qos0(client, topic) != 0) {
        fprintf(stderr, "Failed to subscribe to topic.\n");
        mqtt_client_disconnect(client);
        mqtt_client_destroy(client);
        return 1;
    }

    printf("Listening for messages... Press Ctrl+C to exit.\n");

    // Simple infinite loop: receive and print messages
    while (1) {
        if (mqtt_client_loop(client) != 0) {
            fprintf(stderr, "Error in mqtt_client_loop, exiting.\n");
            break;
        }
    }

    mqtt_client_disconnect(client);
    mqtt_client_destroy(client);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc >= 2 && strcmp(argv[1], "pub") == 0) {
        if (argc < 6) {
            fprintf(stderr, "Usage: %s pub <host> <port> <topic> <message>\n", argv[0]);
            return 1;
        }
        const char *host    = argv[2];
        uint16_t    port    = (uint16_t)atoi(argv[3]);
        const char *topic   = argv[4];
        const char *message = argv[5];

        return run_publish(host, port, topic, message);

    } else if (argc >= 2 && strcmp(argv[1], "sub") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Usage: %s sub <host> <port> <topic>\n", argv[0]);
            return 1;
        }
        const char *host  = argv[2];
        uint16_t    port  = (uint16_t)atoi(argv[3]);
        const char *topic = argv[4];

        return run_subscribe(host, port, topic);

    } else {
        const char *host = "broker.hivemq.com";
        uint16_t    port = 1883;

        if (argc >= 2) host = argv[1];
        if (argc >= 3) port = (uint16_t)atoi(argv[2]);

        return run_connect_test(host, port);
    }
}
