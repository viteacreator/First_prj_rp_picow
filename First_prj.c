#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "pico/cyw43_arch.h"

#include "lwip/tcp.h"
#include "lwip/ip4_addr.h"

// Uncomment to enable verbose USB debug logs.
#define DEBUG 1

#ifdef DEBUG
#define LOGF(...) printf(__VA_ARGS__)
#else
#define LOGF(...) do {} while (0)
#endif

#define ERRF(...) printf(__VA_ARGS__)

// Set your Wi-Fi credentials here.
#define WIFI_SSID     "WiFi"
#define WIFI_PASSWORD "12345678"

#define HTTP_PORT 80
#define MAX_MSG_LEN 96

typedef struct {
    critical_section_t lock;
    uint32_t prime_count;
    uint32_t last_prime;
    uint32_t prime_max;
    int led_manual;
    int blink_ms;
    char last_msg[MAX_MSG_LEN];
} shared_state_t;

static shared_state_t g_state;

static bool is_prime(uint32_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if ((n & 1u) == 0u) return false;
    for (uint32_t i = 3; i * i <= n; i += 2) {
        if ((n % i) == 0u) return false;
    }
    return true;
}

static void core1_main(void) {
    uint32_t n = 2;
    while (true) {
        uint32_t local_max;
        critical_section_enter_blocking(&g_state.lock);
        local_max = g_state.prime_max;
        critical_section_exit(&g_state.lock);

        if (is_prime(n)) {
            critical_section_enter_blocking(&g_state.lock);
            g_state.prime_count++;
            g_state.last_prime = n;
            critical_section_exit(&g_state.lock);
        }

        n++;
        if (n > local_max) {
            n = 2;
        }
        tight_loop_contents();
    }
}

static void url_decode(char *dst, size_t dst_len, const char *src) {
    size_t di = 0;
    for (size_t si = 0; src[si] != '\0' && di + 1 < dst_len; si++) {
        if (src[si] == '%' && src[si + 1] && src[si + 2] &&
            isxdigit((unsigned char)src[si + 1]) &&
            isxdigit((unsigned char)src[si + 2])) {
            char hex[3] = { src[si + 1], src[si + 2], '\0' };
            dst[di++] = (char)strtoul(hex, NULL, 16);
            si += 2;
        } else if (src[si] == '+') {
            dst[di++] = ' ';
        } else {
            dst[di++] = src[si];
        }
    }
    dst[di] = '\0';
}

static void update_state_from_request(const char *path) {
    if (strncmp(path, "/led?state=", 11) == 0) {
        int state = atoi(path + 11);
        critical_section_enter_blocking(&g_state.lock);
        g_state.led_manual = state ? 1 : 0;
        g_state.blink_ms = 0;
        critical_section_exit(&g_state.lock);
        LOGF("LED manual set to %d\n", state ? 1 : 0);
        return;
    }

    if (strncmp(path, "/blink?ms=", 10) == 0) {
        int ms = atoi(path + 10);
        if (ms < 50) ms = 50;
        if (ms > 2000) ms = 2000;
        critical_section_enter_blocking(&g_state.lock);
        g_state.blink_ms = ms;
        critical_section_exit(&g_state.lock);
        LOGF("Blink set to %d ms\n", ms);
        return;
    }

    if (strncmp(path, "/prime?max=", 11) == 0) {
        int max = atoi(path + 11);
        if (max < 100) max = 100;
        if (max > 50000) max = 50000;
        critical_section_enter_blocking(&g_state.lock);
        g_state.prime_max = (uint32_t)max;
        g_state.prime_count = 0;
        g_state.last_prime = 0;
        critical_section_exit(&g_state.lock);
        LOGF("Prime max set to %d\n", max);
        return;
    }

    if (strncmp(path, "/send?msg=", 10) == 0) {
        char decoded[MAX_MSG_LEN];
        url_decode(decoded, sizeof(decoded), path + 10);
        LOGF("WEB MSG: %s\n", decoded);
        critical_section_enter_blocking(&g_state.lock);
        strncpy(g_state.last_msg, decoded, sizeof(g_state.last_msg) - 1);
        g_state.last_msg[sizeof(g_state.last_msg) - 1] = '\0';
        critical_section_exit(&g_state.lock);
        return;
    }
}

static void build_page(char *out, size_t out_len) {
    uint32_t prime_count;
    uint32_t last_prime;
    uint32_t prime_max;
    int led_manual;
    int blink_ms;
    char last_msg[MAX_MSG_LEN];

    critical_section_enter_blocking(&g_state.lock);
    prime_count = g_state.prime_count;
    last_prime = g_state.last_prime;
    prime_max = g_state.prime_max;
    led_manual = g_state.led_manual;
    blink_ms = g_state.blink_ms;
    strncpy(last_msg, g_state.last_msg, sizeof(last_msg) - 1);
    last_msg[sizeof(last_msg) - 1] = '\0';
    critical_section_exit(&g_state.lock);

    snprintf(out, out_len,
        "<!doctype html>"
        "<html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Pico W Control</title>"
        "<style>"
        "body{font-family:Verdana,Arial,sans-serif;background:#f5f2e9;margin:20px;}"
        ".card{background:#fff;border:2px solid #222;padding:16px;margin-bottom:16px;}"
        "button{padding:10px 14px;margin:4px;border:2px solid #222;background:#eae2d0;}"
        "input{padding:8px;border:2px solid #222;}"
        "</style></head><body>"
        "<h2>Pico W Web Console</h2>"
        "<div class='card'>"
        "<div><b>Primes</b>: count=%lu last=%lu max=%lu</div>"
        "<div><b>LED</b>: manual=%d blink_ms=%d</div>"
        "<div><b>Last msg</b>: %s</div>"
        "</div>"
        "<div class='card'>"
        "<button onclick=\"location.href='/led?state=1'\">LED ON</button>"
        "<button onclick=\"location.href='/led?state=0'\">LED OFF</button>"
        "<button onclick=\"location.href='/blink?ms=200'\">Blink 200ms</button>"
        "<button onclick=\"location.href='/blink?ms=700'\">Blink 700ms</button>"
        "</div>"
        "<div class='card'>"
        "<form action='/send' method='get'>"
        "<input name='msg' placeholder='Send to USB'/>"
        "<button type='submit'>Send</button>"
        "</form>"
        "</div>"
        "<div class='card'>"
        "<form action='/prime' method='get'>"
        "<input name='max' placeholder='Prime max (100-50000)'/>"
        "<button type='submit'>Set Prime Max</button>"
        "</form>"
        "</div>"
        "</body></html>",
        (unsigned long)prime_count,
        (unsigned long)last_prime,
        (unsigned long)prime_max,
        led_manual,
        blink_ms,
        last_msg[0] ? last_msg : "(none)");
}

static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    (void)arg;
    (void)err;
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char req[512];
    size_t len = pbuf_copy_partial(p, req, sizeof(req) - 1, 0);
    tcp_recved(tpcb, p->tot_len);
    req[len] = '\0';
    pbuf_free(p);

    const char *path = "/";
    if (strncmp(req, "GET ", 4) == 0) {
        char *start = req + 4;
        char *space = strchr(start, ' ');
        if (space) {
            *space = '\0';
            path = start;
        }
    }

    LOGF("HTTP GET %s\n", path);
    update_state_from_request(path);

    char body[1024];
    build_page(body, sizeof(body));

    char header[128];
    int body_len = (int)strlen(body);
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n\r\n",
             body_len);

    tcp_write(tpcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
    tcp_write(tpcb, body, body_len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    tcp_close(tpcb);
    return ERR_OK;
}

static err_t http_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    (void)arg;
    (void)err;
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

static bool start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) return false;
    if (tcp_bind(pcb, NULL, HTTP_PORT) != ERR_OK) {
        tcp_close(pcb);
        return false;
    }
    struct tcp_pcb *listen_pcb = tcp_listen_with_backlog(pcb, 4);
    if (!listen_pcb) {
        tcp_close(pcb);
        return false;
    }
    pcb = listen_pcb;
    tcp_accept(pcb, http_accept);
    return true;
}

int main(void) {
    stdio_init_all();
    sleep_ms(1500);

    critical_section_init(&g_state.lock);
    g_state.prime_max = 5000;
    g_state.led_manual = 0;
    g_state.blink_ms = 0;
    g_state.last_msg[0] = '\0';

    if (cyw43_arch_init()) {
        ERRF("CYW43 init failed\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    LOGF("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        ERRF("Wi-Fi connect failed\n");
        return 1;
    }

    struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];
    LOGF("Connected. IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));

    cyw43_arch_lwip_begin();
    if (!start_http_server()) {
        cyw43_arch_lwip_end();
        ERRF("HTTP server failed to start\n");
        return 1;
    }
    cyw43_arch_lwip_end();
    LOGF("HTTP server started on port %d\n", HTTP_PORT);

    multicore_launch_core1(core1_main);

    absolute_time_t next_blink = make_timeout_time_ms(200);
    bool led_state = false;
    while (true) {
        int led_manual;
        int blink_ms;
        critical_section_enter_blocking(&g_state.lock);
        led_manual = g_state.led_manual;
        blink_ms = g_state.blink_ms;
        critical_section_exit(&g_state.lock);

        if (blink_ms > 0) {
            if (absolute_time_diff_us(get_absolute_time(), next_blink) <= 0) {
                led_state = !led_state;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
                next_blink = make_timeout_time_ms(blink_ms);
            }
        } else {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_manual ? 1 : 0);
        }

        sleep_ms(10);
    }
}
