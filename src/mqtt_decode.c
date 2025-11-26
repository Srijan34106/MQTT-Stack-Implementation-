#include "mqtt_decode.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int mqtt_decode_connack(const uint8_t *buf, size_t len) {
    if (len < 4) return -1;

    if (buf[0] != 0x20) {
        fprintf(stderr, "Not a CONNACK packet\n");
        return -1;
    }

    // buf[1] = remaining length (expect 2)
    if (buf[3] != 0) {
        fprintf(stderr, "CONNACK error code: %d\n", buf[3]);
        return -1;
    }

    return 0;
}

int mqtt_decode_suback(const uint8_t *buf, size_t len) {
    if (len < 5) {
        fprintf(stderr, "SUBACK too short\n");
        return -1;
    }

    if ((buf[0] & 0xF0) != 0x90) {
        fprintf(stderr, "Not a SUBACK packet\n");
        return -1;
    }

    // buf[1] = remaining length, should be >= 3 (packet id + one return code)
    uint8_t return_code = buf[len - 1];
    if (return_code == 0x80) {
        fprintf(stderr, "Subscription failed (return code 0x80)\n");
        return -1;
    }

    return 0;
}

int mqtt_decode_publish_qos0(const uint8_t *buf, size_t len,
                             char *topic_buf, size_t topic_buf_size,
                             const uint8_t **payload,
                             size_t *payload_len) {
    if (len < 4) return -1;

    uint8_t packet_type = buf[0] >> 4;
    if (packet_type != 3) {
        fprintf(stderr, "Not a PUBLISH packet\n");
        return -1;
    }

    uint8_t remaining_len = buf[1];
    if (len < (size_t)(2 + remaining_len)) {
        fprintf(stderr, "PUBLISH: incomplete packet\n");
        return -1;
    }

    const uint8_t *ptr = &buf[2];
    size_t bytes_left = remaining_len;

    if (bytes_left < 2) return -1;

    uint16_t topic_len = (uint16_t)(ptr[0] << 8) | ptr[1];
    ptr += 2;
    bytes_left -= 2;

    if (bytes_left < topic_len) return -1;
    if (topic_len + 1 > topic_buf_size) {
        fprintf(stderr, "Topic buffer too small\n");
        return -1;
    }

    memcpy(topic_buf, ptr, topic_len);
    topic_buf[topic_len] = '\0';

    ptr += topic_len;
    bytes_left -= topic_len;

    // QoS0 => no packet identifier field, payload starts here
    *payload = ptr;
    *payload_len = bytes_left;

    return 0;
}

