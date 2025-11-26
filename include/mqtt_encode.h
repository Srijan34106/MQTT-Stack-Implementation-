#ifndef MQTT_ENCODE_H
#define MQTT_ENCODE_H

#include <stddef.h>
#include <stdint.h>

/**
 * Encode MQTT CONNECT packet into buffer.
 *
 * @return length of encoded packet, or -1 on error
 */
int mqtt_encode_connect(uint8_t *buf, size_t bufsize,
                        const char *client_id,
                        uint16_t keep_alive);

/**
 * Encode MQTT PUBLISH (QoS 0) packet.
 */
int mqtt_encode_publish_qos0(uint8_t *buf, size_t bufsize,
                             const char *topic,
                             const uint8_t *payload,
                             size_t payload_len);

/**
 * Encode MQTT SUBSCRIBE packet (single topic, QoS 0).
 *
 * Note: MQTT spec requires SUBSCRIBE control packet to use QoS1
 * in the fixed header, even if requested topic QoS is 0.
 */
int mqtt_encode_subscribe_qos0(uint8_t *buf, size_t bufsize,
                               uint16_t packet_id,
                               const char *topic);

#endif // MQTT_ENCODE_H
