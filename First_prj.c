#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip4_addr.h"

#include "debug.h"
#include "state.h"
#include "web_server.h"
#include "prime_worker.h"
#include "wifi_manager.h"
#include "mdns_manager.h"

// Set your Wi-Fi credentials here.
#define WIFI_SSID     "WiFi"
#define WIFI_PASSWORD "12345678"

/* Initialize hardware, connect to Wi-Fi, start services, then run the LED loop. */
int main(void) {
    stdio_init_all();
    sleep_ms(1500);

    state_init();

    if (cyw43_arch_init()) {
        ERRF("CYW43 init failed\n");
        return 1;
    }
    wifi_mode_t mode = wifi_connect_or_start_ap(WIFI_SSID, WIFI_PASSWORD);
    const struct netif *netif = wifi_get_netif(mode);
    LOGF("IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));

    mdns_start(netif, "pico-w");

    if (!start_http_server()) {
        ERRF("HTTP server failed to start\n");
        return 1;
    }
    LOGF("HTTP server started\n");

    prime_worker_start();

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
