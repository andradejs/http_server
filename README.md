
# 🌡️📶 Raspberry Pi Pico W - Sensor e Comunicação HTTP

Este projeto utiliza uma **Raspberry Pi Pico W** para ler dados de:

- Botão digital (GPIO)
- Joystick analógico (eixos X e Y via ADC)
- Temperatura interna (via sensor embutido)

Esses dados são enviados via **HTTP GET** para um servidor, seja local ou em nuvem.

---

## 🛠️ Hardware Requerido

- Raspberry Pi Pico W
- Joystick analógico
- Botão (com resistor de pull-up ou pull-down)
- Conexão Wi-Fi

---

## 📦 Componentes Conectados

| Componente      | Pino da Pico W |
|-----------------|----------------|
| Botão           | GPIO 5         |
| Joystick X      | GPIO 26 (ADC1) |
| Joystick Y      | GPIO 27 (ADC0) |

---

## 📶 Fluxo de Funcionamento

1. A Pico W inicializa os periféricos e conecta-se ao Wi-Fi.
2. A cada segundo, coleta:
   - Estado do botão
   - Leitura dos eixos do joystick
   - Temperatura interna
3. Gera uma URL com os dados como parâmetros de query.
4. Envia a URL via **HTTP GET** ao servidor remoto ou local.

---

## 🌐 Configuração do Servidor

No código você pode usar:

```c
#define HOST "192.168.1.15"        // Servidor local
// #define HOST "app-http-server-rasp.onrender.com"  // Servidor em nuvem
#define URL_REQUEST "/status"
````

A URL completa será parecida com:

```
GET /status?button=1&sensor=25.64&x=2020&y=2111
```

---

## 🧠 Funções Principais no Código

```c
ADCValues read_jostick();
bool read_button();
float read_sensor();
```

Cada uma retorna valores correspondentes ao seu respectivo componente.

---

## 🔐 HTTPS (Opcional)

Caso use HTTPS:

1. Defina o certificado raiz como string `TLS_ROOT_CERT_OK`
2. Descomente no código:

```c
req1.tls_config = altcp_tls_create_config_client(cert_ok, sizeof(cert_ok));
altcp_tls_free_config(req1.tls_config);
```

---

## 📋 Trecho do Loop Principal

```c
while (true) {
    bool status_button = read_button();
    float sensor_data = read_sensor();
    ADCValues values = read_jostick();

    char full_url[50];
    snprintf(full_url, sizeof(full_url),
        "%s?button=%d&sensor=%.2f&x=%d&y=%d",
        URL_REQUEST, status_button, sensor_data, values.x, values.y
    );

    req1.url = full_url;
    int result = http_client_request_sync(cyw43_arch_async_context(), &req1);

    if (result != 0) {
        printf("Falha ao enviar dados ao servidor: %d\n", result);
    }

    cyw43_arch_poll();
    sleep_ms(1000);
}
```

---

## 🧪 Exemplo de Saída no Terminal

```
inicializando raspberry pi pico
status botao 1 
temperatura 25.84 °C
```

---

## 📜 Licença

Distribuído sob licença **BSD-3-Clause**.
Consulte o cabeçalho do arquivo fonte para mais informações.

---

## ✍️ Autor

Projeto desenvolvido para aprendizado de IoT com **Raspberry Pi Pico W**.

