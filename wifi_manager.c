#include "pico/cyw43_arch.h"

#include "lwip/ip4_addr.h"

#include "debug.h"
#include "wifi_manager.h"

/* Connect to Wi-Fi as a station, or fall back to AP mode with the same SSID. */
wifi_mode_t wifi_connect_or_start_ap(const char *ssid, const char *password) {
    cyw43_arch_enable_sta_mode();
    LOGI("Connecting to Wi-Fi SSID: %s\n", ssid);
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000) == 0) {
        LOGI("Wi-Fi station connected\n");
        return WIFI_MODE_STA;
    }

    LOGW("Wi-Fi connect failed. Starting AP with SSID: %s\n", ssid);
    cyw43_arch_enable_ap_mode(ssid, password, CYW43_AUTH_WPA2_AES_PSK);
    LOGW("AP mode started. DHCP server not available in this SDK install.\n");
    LOGW("Set phone IP manually: 192.168.4.2/24, gateway: 192.168.4.1\n");
    return WIFI_MODE_AP;
}

/* Get the correct lwIP network interface for the active Wi-Fi mode. */
const struct netif *wifi_get_netif(wifi_mode_t mode) {
    if (mode == WIFI_MODE_AP) {
        LOGD("wifi_get_netif: AP\n");
        return &cyw43_state.netif[CYW43_ITF_AP];
    }
    LOGD("wifi_get_netif: STA\n");
    return &cyw43_state.netif[CYW43_ITF_STA];
}
