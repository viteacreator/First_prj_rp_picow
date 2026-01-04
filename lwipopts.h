#pragma once

// Minimal lwIP options for Pico W HTTP server example.
#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0
#define LWIP_STATS 0

#define LWIP_TCP 1
#define LWIP_UDP 1

#define LWIP_IPV4 1
#define LWIP_IPV6 0
#define LWIP_IGMP 1

#define LWIP_DHCP 1
#define LWIP_DNS 1
#define DNS_TABLE_SIZE 1

#define LWIP_MDNS_RESPONDER 1
#define LWIP_MDNS_SEARCH 0
#define MDNS_MAX_SERVICES 1

#define LWIP_NETIF_HOSTNAME 1
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1
#define LWIP_NETIF_CLIENT_DATA 1
#define LWIP_NUM_NETIF_CLIENT_DATA 1

#define MEM_LIBC_MALLOC 0
#define MEM_ALIGNMENT 4
#define MEM_SIZE (8 * 1024)

#define MEMP_NUM_TCP_PCB 5
#define MEMP_NUM_TCP_SEG 32
#define MEMP_NUM_SYS_TIMEOUT 32

#define PBUF_POOL_SIZE 24
#define PBUF_POOL_BUFSIZE 1520

#define TCP_MSS 1460
#define TCP_SND_BUF (2 * TCP_MSS)
#define TCP_WND (2 * TCP_MSS)

#define LWIP_HTTPD 0
