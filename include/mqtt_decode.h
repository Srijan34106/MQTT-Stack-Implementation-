#ifndef MQTT_DECODE_H
#define MQTT_DECODE_H

#include <stddef.h>
#include <stdint.h>

/**
 * Decode MQTT CONNACK packet.
 *
 * @return 0 = success, non-zero = failure
 */
int mqtt_decode_connack(const uint8_t *buf, size_t len);

/**
 * Decode MQTT SUBACK for single topic.
 *
 * @return 0 = success, non-zero = failure
 */
int mqtt_decode_suback(const uint8_t *buf, size_t len);

/**
 * Decode MQTT PUBLISH (QoS 0) packet.
 *
 * topic_buf will be null-terminated on success.
 *
 * @return 0 = success, non-zero = failure
 */
int mqtt_decode_publish_qos0(const uint8_t *buf, size_t len,
                             char *topic_buf, size_t topic_buf_size,
                             const uint8_t **payload,
                             size_t *payload_len);

#endif // MQTT_DECODE_H

