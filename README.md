MQTT v3.1.1 Client Stack in C (From Scratch)

This repository contains a from-scratch implementation of an MQTT v3.1.1 client written in C — without using any external MQTT libraries.  
The project focuses on embedded-systems friendly architecture and currently runs on Linux (POSIX sockets), while being easily portable to STM32 / ESP32 / FreeRTOS / Bare-metal.

---

## Features Implemented

| Feature | Status |
|--------|--------|
| TCP transport layer | ✅ |
| MQTT CONNECT / CONNACK | ✅ |
| MQTT PUBLISH (QoS 0) | ✅ |
| MQTT SUBSCRIBE / SUBACK (single topic) | ✅ |
| Receive messages via callback | ✅ |
| CLI application for testing | ✅ |



