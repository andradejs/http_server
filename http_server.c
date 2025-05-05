/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"
#include "example_http_client_util.h"
#include "hardware/adc.h"
 

#define HOST "app-http-server-rasp.onrender.com"
#define URL_REQUEST "/status"

#define BUTTON1 5
#define JOYSTICK_X 26
#define JOYSTICK_Y 27

// This is the PUBLIC root certificate exported from a browser
 // Note that the newlines are needed
 #define TLS_ROOT_CERT_OK "-----BEGIN CERTIFICATE-----\n\
MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n\
VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n\
A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n\
WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n\
IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n\
AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n\
QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n\
HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n\
BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n\
9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n\
p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n\
-----END CERTIFICATE-----\n"
 


typedef struct {
    uint16_t x;
    uint16_t y;
} ADCValues;

ADCValues read_jostick(){

    ADCValues values;
    adc_select_input(1);
    values.x = adc_read();

    adc_select_input(0);
    values.y = adc_read();

    return values;
}

bool read_button()
{
    bool status = !gpio_get(BUTTON1);
    return status;
}

float read_sensor()
{
    // Configura o ADC
    adc_select_input(4);
    uint16_t adc_raw = adc_read();
    
    // Converter para tensão
    float adc_voltage = (adc_raw / 4095.0f) * 3.3f;
    
    // Calcular a temperatura
    float temperature = 27.0f - (adc_voltage - 0.706f) / 0.001721f;
    
    // Exibir a temperatur
    
    return temperature;
}




int main()
{
    stdio_init_all();
    sleep_ms(2000);
    printf("inicializando raspberry pi pico");
    
    gpio_init(BUTTON1);
    gpio_set_dir(BUTTON1, false);
    gpio_pull_up(BUTTON1);
    
    adc_init();
    adc_set_temp_sensor_enabled(true);
  

    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("failed to connect\n");
        return 1;
    }


    static const uint8_t cert_ok[] = TLS_ROOT_CERT_OK;
    EXAMPLE_HTTP_REQUEST_T req1 = {0};
    req1.hostname = HOST;
    req1.port = 443,
    req1.headers_fn = http_client_header_print_fn; 
    req1.recv_fn = http_client_receive_print_fn;
    // req1.tls_config = altcp_tls_create_config_client(cert_ok, sizeof(cert_ok));
    // altcp_tls_free_config(req1.tls_config);


    while (true)
    {
        
        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
            printf("Conexão Wi-Fi perdida, tentando reconectar...\n");
            while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
                printf("Tentando reconectar ao Wi-Fi...\n");
                sleep_ms(5000);
            }
            printf("Reconectado ao Wi-Fi\n");
        }

        bool status_button = read_button();
        float sensor_data = read_sensor();
        ADCValues values = read_jostick();

        char full_url[50];

        snprintf(full_url, sizeof(full_url), "%s?button=%d&sensor=%.2f&x=%d&y=%d",URL_REQUEST,status_button,sensor_data,values.x,values.y);

        req1.url = full_url;
        printf("status botao %d \n", status_button);
        printf("temperatura %.2f °C\n", sensor_data);

        req1.tls_config = altcp_tls_create_config_client(cert_ok, sizeof(cert_ok));
        int result = http_client_request_sync(cyw43_arch_async_context(),&req1);
        altcp_tls_free_config(req1.tls_config);

         if (result != 0){
             printf("Falha ao enviar dados ao servidor:%d \n", result);
        }

        // Processa o contexto assíncrono regularmente
        // async_context_poll(cyw43_arch_async_context());
        cyw43_arch_poll();
        // tight_loop_contents();
        sleep_ms(500);
    }

    cyw43_arch_deinit();
    return 0;
}