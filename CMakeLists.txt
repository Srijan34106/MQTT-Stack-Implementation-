cmake_minimum_required(VERSION 3.10)

project(mqtt_client C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(mqtt_cli
    src/mqtt_client.c
    src/mqtt_transport_posix.c
    src/mqtt_encode.c
    src/mqtt_decode.c
    examples/mqtt_cli.c
)
