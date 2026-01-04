#pragma once

#include <stdbool.h>
#include "lwip/netif.h"

/* Start the mDNS responder so the device is reachable at <hostname>.local. */
bool mdns_start(const struct netif *netif, const char *hostname);
