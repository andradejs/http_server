# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Inicializar pico_sdk

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Incluir o Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(http_server C CXX ASM)

pico_sdk_init()

pico_add_library(example_lwip_http_util NOFLAG)
target_sources(example_lwip_http_util INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/example_http_client_util.c
        )
pico_mirrored_target_link_libraries(example_lwip_http_util INTERFACE
        pico_lwip_http
        pico_lwip_mbedtls
        pico_mbedtls
        )
target_include_directories(example_lwip_http_util INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        )

# Adicionar executável
add_executable(http_server http_server.c)
set(WIFI_SSID "Online Rose")
set(WIFI_PASSWORD "52074185")
target_compile_definitions(http_server PRIVATE
        WIFI_SSID="${WIFI_SSID}"
        WIFI_PASSWORD="${WIFI_PASSWORD}"
        )

pico_set_program_name(http_server "http_server")
pico_set_program_version(http_server "0.1")

# Configuração de saída
pico_enable_stdio_uart(http_server 0)
pico_enable_stdio_usb(http_server 1)

target_link_libraries(http_server
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        example_lwip_http_util
        hardware_adc
        )

target_include_directories(http_server PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/..
)

pico_add_extra_outputs(http_server)