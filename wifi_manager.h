#pragma once

#include "lwip/netif.h"

typedef enum {
    WIFI_MODE_STA = 0,
    WIFI_MODE_AP = 1
} wifi_mode_t;

/* Try to connect as station; if it fails, start an AP with the same credentials. */
wifi_mode_t wifi_connect_or_start_ap(const char *ssid, const char *password);

/* Return the lwIP netif for the current mode (STA or AP). */
const struct netif *wifi_get_netif(wifi_mode_t mode);
