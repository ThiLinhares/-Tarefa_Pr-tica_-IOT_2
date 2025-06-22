#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"

// --- Configurações ---
#define WIFI_SSID "@thilinhares"    //  <--- COLOQUE AQUI O NOME DA SUA REDE WIFI
#define WIFI_PASSWORD "mafredzudo18" //  <--- COLOQUE AQUI A SENHA DA SUA REDE WIFI

#define JOYSTICK_VRY_PIN 26 // ADC0
#define JOYSTICK_VRX_PIN 27 // ADC1
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// --- Tipos e Variáveis Globais ---
typedef enum {
    CENTER, NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST
} joystick_direction_t;

typedef enum {
    NO_BUTTON_PRESSED_YET, BUTTON_A_EVENT, BUTTON_B_EVENT
} button_event_type_t;

volatile uint16_t g_joystick_vrx_value = 0;
volatile uint16_t g_joystick_vry_value = 0;
volatile joystick_direction_t g_joystick_direction = CENTER;
volatile button_event_type_t g_last_button_type = NO_BUTTON_PRESSED_YET;

#define ADC_MAX_VALUE 4095
#define ADC_CENTER_VALUE (ADC_MAX_VALUE / 2)
#define JOYSTICK_DEADZONE 300
#define JOYSTICK_LOW_THRESHOLD  (ADC_CENTER_VALUE - JOYSTICK_DEADZONE)
#define JOYSTICK_HIGH_THRESHOLD (ADC_CENTER_VALUE + JOYSTICK_DEADZONE)

#define TCP_PORT 80
#define HTTP_SERVER_BACKLOG 5 // Número de conexões TCP pendentes que o servidor pode enfileirar

// --- Protótipos ---
static err_t tcp_server_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static void tcp_server_error_callback(void *arg, err_t err);
const char* joystick_direction_to_string(joystick_direction_t dir);
const char* joystick_direction_to_css_class(joystick_direction_t dir);
static bool initialize_tcp_server(void);
static void read_inputs_and_update_state(void);

// --- Implementações ---

static void tcp_server_error_callback(void *arg, err_t err) {
    // ERR_ABRT é comum quando abortamos a conexão intencionalmente ou o cliente fecha abruptamente.
    if (err != ERR_ABRT) {
        printf("ERRO TCP: Callback de erro TCP - código %d no PCB %p\n", err, arg);
    }
}

const char* joystick_direction_to_string(joystick_direction_t dir) {
    switch (dir) {
        case NORTH:     return "Norte";
        case NORTHEAST: return "Nordeste";
        case EAST:      return "Leste";
        case SOUTHEAST: return "Sudeste";
        case SOUTH:     return "Sul";
        case SOUTHWEST: return "Sudoeste";
        case WEST:      return "Oeste";
        case NORTHWEST: return "Noroeste";
        case CENTER:    return "Centro";
        default:        return "Desconhecido";
    }
}

const char* joystick_direction_to_css_class(joystick_direction_t dir) {
    switch (dir) {
        case NORTH:     return "dir-n";
        case NORTHEAST: return "dir-ne";
        case EAST:      return "dir-e";
        case SOUTHEAST: return "dir-se";
        case SOUTH:     return "dir-s";
        case SOUTHWEST: return "dir-sw";
        case WEST:      return "dir-w";
        case NORTHWEST: return "dir-nw";
        case CENTER:    return "dir-center";
        default:        return "dir-unknown";
    }
}

// Callback para quando dados são recebidos em uma conexão TCP
static err_t tcp_server_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (err != ERR_OK && err != ERR_ABRT) {
        printf("ERRO TCP: Falha na recepção de dados - código %d\n", err);
        if (p != NULL) {
            tcp_recved(tpcb, p->tot_len);
            pbuf_free(p);
        }
        tcp_arg(tpcb, NULL); tcp_recv(tpcb, NULL); tcp_sent(tpcb, NULL); tcp_poll(tpcb, NULL, 0); tcp_err(tpcb, NULL);
        tcp_abort(tpcb); // Aborta a conexão problemática
        return ERR_ABRT;
    }

    // Se p == NULL, o cliente fechou a conexão
    if (p == NULL) {
        // printf("INFO: Conexão TCP fechada pelo cliente.\n"); 
        return ERR_OK; // Indica que lidamos com o fechamento
    }

    tcp_recved(tpcb, p->tot_len); // Informa à LwIP que processamos os dados

    bool serve_page = false;
    if (p->len >= 3 && strncmp((char*)p->payload, "GET", 3) == 0) {
        serve_page = true;
    }
    pbuf_free(p); // Libera o buffer da requisição, não precisamos mais dele

    if (!serve_page) {
        err_t close_err = tcp_close(tpcb);
        if (close_err != ERR_OK) {
            printf("ERRO TCP: Falha ao fechar conexão não-GET - código %d. Abortando.\n", close_err);
            tcp_arg(tpcb, NULL); tcp_recv(tpcb, NULL); tcp_sent(tpcb, NULL); tcp_poll(tpcb, NULL, 0); tcp_err(tpcb, NULL);
            tcp_abort(tpcb);
            return ERR_ABRT;
        }
        return ERR_OK;
    }

    // --- Preparar dados para a página HTML ---
    adc_select_input(4); // Sensor de temperatura interno
    uint16_t raw_temp_value = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    float temperature = 27.0f - ((raw_temp_value * conversion_factor) - 0.706f) / 0.001721f;

    char temp_color_str[10];
    if (temperature < 55.0f) strcpy(temp_color_str, "green");
    else if (temperature <= 70.0f) strcpy(temp_color_str, "orange");
    else strcpy(temp_color_str, "red");

    char joystick_direction_text_str[60];
    char joystick_values_str[70];
    char joystick_css_class_str[20];

    snprintf(joystick_direction_text_str, sizeof(joystick_direction_text_str), "Direção: %s", joystick_direction_to_string(g_joystick_direction));
    strncpy(joystick_css_class_str, joystick_direction_to_css_class(g_joystick_direction), sizeof(joystick_css_class_str) -1);
    joystick_css_class_str[sizeof(joystick_css_class_str)-1] = '\0';
    snprintf(joystick_values_str, sizeof(joystick_values_str), "Valores: VRx=%d, VRy=%d", g_joystick_vrx_value, g_joystick_vry_value);

    char button_str[80];
    if (g_last_button_type == BUTTON_A_EVENT) snprintf(button_str, sizeof(button_str), "Ultimo botão pressionado: A");
    else if (g_last_button_type == BUTTON_B_EVENT) snprintf(button_str, sizeof(button_str), "Ultimo botão pressionado: B");
    else snprintf(button_str, sizeof(button_str), "Ultimo botão pressionado: Nenhum");

    // Buffer para o HTML. Ajuste conforme necessário.
    char html[3072]; 
    int len = snprintf(html, sizeof(html),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\n\r\n"
        "<!DOCTYPE html>"
        "<html><head><title>RP2040 Status</title>" // Título da aba do navegador
        "<style>"
        "body { text-align: center; font-family: Arial, sans-serif; margin-top: 20px; background-color: #f4f4f4; color: #333; }"
        "h1 { font-size: 2em; margin-bottom: 25px; color: #0056b3; }"
        "p { font-size: 1.4em; margin: 15px 0; line-height: 1.6; }"
        ".joystick-details { font-size: 0.9em; color: #555; margin-top: 5px; margin-bottom: 15px; }"
        "hr { width: 70%%; margin: 25px auto; border: 0; height: 1px; background-color: #cccccc; }"
        ".container { background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); display: inline-block; }"
        ".compass { width: 100px; height: 100px; border: 2px solid #aaa; border-radius: 50%%; position: relative; margin: 20px auto 10px auto; background-color: #e9e9e9; }"
        ".compass-arrow { width: 0; height: 0; border-left: 10px solid transparent; border-right: 10px solid transparent; border-bottom: 35px solid #d9534f; position: absolute; top: 15px; left: 50%%; transform-origin: 50%% 80%%; transform: translateX(-50%%) rotate(0deg); transition: transform 0.3s ease-out; }"
        ".compass-center-dot { width: 10px; height: 10px; background-color: #333; border-radius: 50%%; position: absolute; top: 50%%; left: 50%%; transform: translate(-50%%, -50%%); display: none; }"
        ".compass.dir-n .compass-arrow { display: block; transform: translateX(-50%%) rotate(0deg); }"
        ".compass.dir-ne .compass-arrow { display: block; transform: translateX(-50%%) rotate(45deg); }"
        ".compass.dir-e .compass-arrow { display: block; transform: translateX(-50%%) rotate(90deg); }"
        ".compass.dir-se .compass-arrow { display: block; transform: translateX(-50%%) rotate(135deg); }"
        ".compass.dir-s .compass-arrow { display: block; transform: translateX(-50%%) rotate(180deg); }"
        ".compass.dir-sw .compass-arrow { display: block; transform: translateX(-50%%) rotate(225deg); }"
        ".compass.dir-w .compass-arrow { display: block; transform: translateX(-50%%) rotate(270deg); }"
        ".compass.dir-nw .compass-arrow { display: block; transform: translateX(-50%%) rotate(315deg); }"
        ".compass.dir-center .compass-arrow { display: none; }" 
        ".compass.dir-center .compass-center-dot { display: block; }"
        "</style>"
        "</head><body>"
        "<div class='container'>"
        "<h1>RP 2040 - BitDogLab</h1>" // Título principal da página 
        "<p style=\"color: %s;\">Temperatura interna: %.2f °C</p>"
        "<hr>"
        "<p>%s</p>"
        "<hr>"
        "<div class=\"compass %s\"><div class=\"compass-arrow\"></div><div class=\"compass-center-dot\"></div></div>"
        "<p style=\"margin-top: 0px;\">%s</p>"
        "<p class='joystick-details'>%s</p>"
        "</div>"
        "<script>setTimeout(function(){ window.location.reload(true); }, 1000);</script>"
        "</body></html>",
        temp_color_str, temperature, button_str, joystick_css_class_str, joystick_direction_text_str, joystick_values_str
    );

    if (len < 0 || len >= sizeof(html)) {
        printf("ERRO FATAL: Overflow do buffer HTML! len: %d, size: %u\n", len, sizeof(html));
        // Em caso de overflow, envia uma resposta de erro mínima.
        const char *error_msg = "HTTP/1.1 500 Internal Server Error\r\nContent-Length:0\r\nConnection:close\r\n\r\n";
        tcp_write(tpcb, error_msg, strlen(error_msg), TCP_WRITE_FLAG_COPY);
        tcp_arg(tpcb, NULL); tcp_recv(tpcb, NULL); tcp_sent(tpcb, NULL); tcp_poll(tpcb, NULL, 0); tcp_err(tpcb, NULL);
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
   

    err_t write_err = tcp_write(tpcb, html, len, TCP_WRITE_FLAG_COPY);
    if (write_err != ERR_OK) {
        printf("ERRO TCP: Falha ao enviar dados (tcp_write) - código %d\n", write_err);
        tcp_arg(tpcb, NULL); tcp_recv(tpcb, NULL); tcp_sent(tpcb, NULL); tcp_poll(tpcb, NULL, 0); tcp_err(tpcb, NULL);
        tcp_abort(tpcb);
        return ERR_ABRT;
    }

    err_t output_err = tcp_output(tpcb);
    if (output_err != ERR_OK) {
        printf("ERRO TCP: Falha ao despachar dados (tcp_output) - código %d\n", output_err);
        tcp_arg(tpcb, NULL); tcp_recv(tpcb, NULL); tcp_sent(tpcb, NULL); tcp_poll(tpcb, NULL, 0); tcp_err(tpcb, NULL);
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    
    err_t close_err = tcp_close(tpcb);
    if (close_err != ERR_OK) {
        printf("ERRO TCP: Falha ao fechar conexão (tcp_close) - código %d. Abortando.\n", close_err);
        tcp_arg(tpcb, NULL); tcp_recv(tpcb, NULL); tcp_sent(tpcb, NULL); tcp_poll(tpcb, NULL, 0); tcp_err(tpcb, NULL);
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    
    return ERR_OK;
}

// Callback para quando uma nova conexão TCP é aceita pelo servidor
static err_t tcp_server_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || newpcb == NULL) {
        printf("ERRO TCP: Falha ao aceitar nova conexão - código %d\n", err);
        return ERR_VAL; // Indica um erro
    }

    tcp_setprio(newpcb, TCP_PRIO_NORMAL); // Define prioridade da conexão
    tcp_arg(newpcb, newpcb); // Argumento para os callbacks (geralmente o próprio PCB)
    tcp_recv(newpcb, tcp_server_recv_callback); // Define callback para dados recebidos
    tcp_err(newpcb, tcp_server_error_callback); // Define callback para erros na conexão
    
    return ERR_OK; // Sucesso
}

// Inicializa os componentes do servidor TCP
static bool initialize_tcp_server(void) {
    printf("INFO: Configurando servidor TCP...\n");
    struct tcp_pcb *pcb_listen = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb_listen) {
        printf("ERRO FATAL: Falha ao criar PCB para escuta TCP.\n");
        return false;
    }

    err_t bind_err = tcp_bind(pcb_listen, IP_ANY_TYPE, TCP_PORT);
    if (bind_err != ERR_OK) {
        printf("ERRO FATAL: Falha ao associar (bind) servidor TCP à porta %d - código %d\n", TCP_PORT, bind_err);
        tcp_close(pcb_listen);
        return false;
    }

    pcb_listen = tcp_listen_with_backlog(pcb_listen, HTTP_SERVER_BACKLOG);
    if (!pcb_listen) {
        printf("ERRO FATAL: Falha ao colocar servidor TCP em modo de escuta.\n");
        // O PCB original pode ter sido desalocado em caso de erro aqui
        return false;
    }

    tcp_accept(pcb_listen, tcp_server_accept_callback);
    printf("INFO: Servidor TCP ouvindo na porta %d.\n", TCP_PORT);
    return true;
}

// Lê os pinos de entrada e atualiza o estado global
static void read_inputs_and_update_state(void) {
    adc_select_input(0); // ADC0 (GPIO26) para VRY
    g_joystick_vry_value = adc_read();
    adc_select_input(1); // ADC1 (GPIO27) para VRX
    g_joystick_vrx_value = adc_read();

    bool is_south = (g_joystick_vry_value < JOYSTICK_LOW_THRESHOLD);
    bool is_north = (g_joystick_vry_value > JOYSTICK_HIGH_THRESHOLD);
    bool is_west  = (g_joystick_vrx_value < JOYSTICK_LOW_THRESHOLD);
    bool is_east  = (g_joystick_vrx_value > JOYSTICK_HIGH_THRESHOLD);

    if (is_north && is_west) g_joystick_direction = NORTHWEST;
    else if (is_north && is_east) g_joystick_direction = NORTHEAST;
    else if (is_south && is_west) g_joystick_direction = SOUTHWEST;
    else if (is_south && is_east) g_joystick_direction = SOUTHEAST;
    else if (is_north) g_joystick_direction = NORTH;
    else if (is_south) g_joystick_direction = SOUTH;
    else if (is_west) g_joystick_direction = WEST;
    else if (is_east) g_joystick_direction = EAST;
    else g_joystick_direction = CENTER;
    
    // Detecção de borda de descida para os botões (pressionamento)
    // Assumindo pull-ups internos, botões são ativos em nível baixo.
    static bool prev_button_a_state = true; // Inicializa como não pressionado
    static bool prev_button_b_state = true; // Inicializa como não pressionado

    bool current_button_a_state = gpio_get(BUTTON_A_PIN);
    bool current_button_b_state = gpio_get(BUTTON_B_PIN);

    if (!current_button_a_state && prev_button_a_state) {
        g_last_button_type = BUTTON_A_EVENT;
         printf("DEBUG: Botão A pressionado.\n"); // Opcional para depuração
    }
    if (!current_button_b_state && prev_button_b_state) {
        g_last_button_type = BUTTON_B_EVENT;
         printf("DEBUG: Botão B pressionado.\n"); // Opcional para depuração
    }
    
    prev_button_a_state = current_button_a_state;
    prev_button_b_state = current_button_b_state;
}


int main() {
    stdio_init_all();
    printf("INFO: Iniciando programa RP2040 - BitDogLab Web Controller...\n");

    printf("INFO: Inicializando GPIOs para botões e ADC...\n");
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    
    adc_init();
    adc_gpio_init(JOYSTICK_VRY_PIN);
    adc_gpio_init(JOYSTICK_VRX_PIN);
    adc_set_temp_sensor_enabled(true);
    printf("INFO: Periféricos GPIO e ADC inicializados.\n");

    printf("INFO: Inicializando Wi-Fi...\n");
    if (cyw43_arch_init()) {
        printf("ERRO FATAL: Falha ao inicializar arquitetura Wi-Fi (cyw43_arch).\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();
    printf("INFO: Modo Wi-Fi Station habilitado. Conectando a '%s'...\n", WIFI_SSID);

    int connect_status = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (connect_status) {
        printf("ERRO FATAL: Falha ao conectar ao Wi-Fi. Código: %d\n", connect_status);
        printf("          Verifique SSID, senha e alcance do roteador.\n");
        cyw43_arch_deinit();
        return -1;
    }
    printf("INFO: Wi-Fi conectado com sucesso!\n");
    if (netif_default) {
        printf("INFO: Endereço IP: %s\n", ipaddr_ntoa(netif_ip_addr4(netif_default)));
    } else {
        printf("AVISO: Interface de rede padrão não encontrada. IP não disponível.\n");
    }

    if (!initialize_tcp_server()) {
        cyw43_arch_deinit();
        return -1;
    }
    
    printf("INFO: Configuração completa. Aguardando interações e conexões HTTP...\n");
    if (netif_default) { // Mensagem para o usuário acessar
         printf("      Acesse: http://%s\n", ipaddr_ntoa(netif_ip_addr4(netif_default)));
    }

    // Loop principal da aplicação
    while (true) {
        cyw43_arch_poll(); // Processa eventos de rede e outras tarefas da arquitetura cyw43
        read_inputs_and_update_state(); // Lê sensores e botões
        sleep_ms(50); // Pequeno delay para reduzir carga da CPU e debounce simples
    }

    // Código abaixo normalmente não é alcançado em um sistema embarcado
    printf("INFO: Saindo do loop principal (evento inesperado).\n");
    cyw43_arch_deinit();
    return 0;
}