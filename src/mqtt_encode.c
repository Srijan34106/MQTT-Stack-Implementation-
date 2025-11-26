#include "mqtt_encode.h"
#include <stdint.h>
#include <string.h>

/* Helper: write MQTT UTF-8 string (2-byte length prefix) */
static uint8_t *encode_string(uint8_t *ptr, const char *str) {
    size_t len = strlen(str);
    *ptr++ = (uint8_t)(len >> 8);
    *ptr++ = (uint8_t)(len & 0xFF);
    memcpy(ptr, str, len);
    return ptr + len;
}

/* Remaining length encoder (only supports <128 for simplicity). */
static int encode_remaining_length(uint8_t *ptr, size_t remaining_len) {
    if (remaining_len > 127) {
        return -1; // simple client, no multi-byte remaining length
    }
    *ptr = (uint8_t)remaining_len;
    return 1;
}

int mqtt_encode_connect(uint8_t *buf, size_t bufsize,
                        const char *client_id,
                        uint16_t keep_alive) {

    const char *protocol_name = "MQTT";
    uint8_t protocol_level = 4; // MQTT v3.1.1
    uint8_t connect_flags = 0;  // no will, no username, no password

    size_t payload_len = 2 + strlen(client_id); // length-prefix + client_id
    size_t vh_len      = 10; // protocol name (2+4) + level + flags + keep_alive(2)
    size_t remaining_len = vh_len + payload_len;

    if (remaining_len > 127) return -1;
    if (bufsize < remaining_len + 2) return -1;

    uint8_t *ptr = buf;

    // Fixed header
    *ptr++ = 0x10; // CONNECT
    ptr   += encode_remaining_length(ptr, remaining_len);

    // Variable header
    ptr = encode_string(ptr, protocol_name); // Protocol Name
    *ptr++ = protocol_level;                // Protocol Level
    *ptr++ = connect_flags;                 // Connect Flags
    *ptr++ = (uint8_t)(keep_alive >> 8);    // Keep Alive MSB
    *ptr++ = (uint8_t)(keep_alive & 0xFF);  // Keep Alive LSB

    // Payload (Client ID)
    ptr = encode_string(ptr, client_id);

    return (int)(ptr - buf);
}

int mqtt_encode_publish_qos0(uint8_t *buf, size_t bufsize,
                             const char *topic,
                             const uint8_t *payload,
                             size_t payload_len) {

    size_t topic_len    = strlen(topic);
    size_t vh_len       = 2 + topic_len; // topic length prefix + topic
    size_t remaining_len = vh_len + payload_len;

    if (remaining_len > 127) return -1;
    if (bufsize < remaining_len + 2) return -1;

    uint8_t *ptr = buf;

    // Fixed header: PUBLISH, QoS 0, DUP=0, RETAIN=0
    *ptr++ = 0x30; // 0011 0000

    ptr += encode_remaining_length(ptr, remaining_len);

    // Variable header: Topic Name
    ptr = encode_string(ptr, topic);

    // Payload
    if (payload_len > 0 && payload != NULL) {
        memcpy(ptr, payload, payload_len);
        ptr += payload_len;
    }

    return (int)(ptr - buf);
}

int mqtt_encode_subscribe_qos0(uint8_t *buf, size_t bufsize,
                               uint16_t packet_id,
                               const char *topic) {

    size_t topic_len    = strlen(topic);
    size_t payload_len  = 2 + topic_len + 1; // topic string + requested QoS
    size_t vh_len       = 2;                 // packet identifier
    size_t remaining_len = vh_len + payload_len;

    if (remaining_len > 127) return -1;
    if (bufsize < remaining_len + 2) return -1;

    uint8_t *ptr = buf;

    // Fixed header: SUBSCRIBE (1000), QoS1 (0010) => 1000 0010 => 0x82
    *ptr++ = 0x82;
    ptr   += encode_remaining_length(ptr, remaining_len);

    // Variable header: Packet Identifier
    *ptr++ = (uint8_t)(packet_id >> 8);
    *ptr++ = (uint8_t)(packet_id & 0xFF);

    // Payload: Topic Filter + Requested QoS
    ptr = encode_string(ptr, topic);
    *ptr++ = 0x00; // Requested QoS 0

    return (int)(ptr - buf);
}

