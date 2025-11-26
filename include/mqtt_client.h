#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declaration of internal struct
typedef struct mqtt_client mqtt_client_t;

/**
 * Callback for incoming PUBLISH messages (QoS 0).
 */
typedef void (*mqtt_message_callback_t)(const char *topic,
                                        const uint8_t *payload,
                                        size_t payload_len);

/**
 * Configuration for the MQTT client.
 */
typedef struct {
    const char *host;          // e.g. "broker.hivemq.com"
    uint16_t    port;          // e.g. 1883
    const char *client_id;     // e.g. "srijan-mqtt-client"
    uint16_t    keep_alive_sec;

    const char *username;      // optional
    const char *password;      // optional

    mqtt_message_callback_t on_message; // can be NULL
} mqtt_client_config_t;

mqtt_client_t *mqtt_client_create(const mqtt_client_config_t *cfg);
void mqtt_client_destroy(mqtt_client_t *client);

int  mqtt_client_connect(mqtt_client_t *client);
void mqtt_client_disconnect(mqtt_client_t *client);

int  mqtt_client_loop(mqtt_client_t *client);

/**
 * Publish a QoS 0 message.
 */
int mqtt_client_publish_qos0(mqtt_client_t *client,
                             const char *topic,
                             const uint8_t *payload,
                             size_t payload_len);

/**
 * Subscribe to a topic with QoS 0.
 */
int mqtt_client_subscribe_qos0(mqtt_client_t *client,
                               const char *topic);

#endif // MQTT_CLIENT_H
