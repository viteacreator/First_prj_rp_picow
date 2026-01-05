#include "pico/cyw43_arch.h"

#include "lwip/apps/mdns.h"

#include "debug.h"
#include "mdns_manager.h"

/* Initialize the lwIP mDNS responder and advertise the HTTP service. */
bool mdns_start(const struct netif *netif, const char *hostname) {
    if (!netif || !hostname || !hostname[0]) {
        ERRF("mDNS: invalid parameters\n");
        return false;
    }

    LOGI("mDNS: init for host %s\n", hostname);
    cyw43_arch_lwip_begin();
    mdns_resp_init();
    if (mdns_resp_add_netif((struct netif *)netif, hostname) != ERR_OK) {
        cyw43_arch_lwip_end();
        ERRF("mDNS: failed to add netif\n");
        return false;
    }

    mdns_resp_add_service((struct netif *)netif, hostname, "_http",
                          DNSSD_PROTO_TCP, 80, NULL, NULL);
    mdns_resp_announce((struct netif *)netif);
    cyw43_arch_lwip_end();

    LOGI("mDNS: http://%s.local\n", hostname);
    return true;
}
