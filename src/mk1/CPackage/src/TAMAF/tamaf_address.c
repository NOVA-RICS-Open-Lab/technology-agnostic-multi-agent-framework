#include "TAMAF/tamaf_address.h"
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define strdup _strdup
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#endif

tamaf_address_t* tamaf_address_create(const char* ip, int port) {
    tamaf_address_t* addr = (tamaf_address_t*)malloc(sizeof(tamaf_address_t));
    if (!addr) return NULL;
    addr->ip = strdup(ip);
    addr->port = port;
    return addr;
}

void tamaf_address_destroy(tamaf_address_t* address) {
    if (address) { if (address->ip) free(address->ip); free(address); }
}

void tamaf_address_set_port(tamaf_address_t* address, int port) {
    if (address) address->port = port;
}

char* tamaf_address_get_string(const tamaf_address_t* address) {
    if (!address) return NULL;
    char port_buf[16];
    if (address->port == TAMAF_PORT_NONE) strcpy(port_buf, "None");
    else sprintf(port_buf, "%d", address->port);
    size_t len = strlen(address->ip) + 1 + strlen(port_buf) + 1;
    char* str = (char*)malloc(len);
    if (str) sprintf(str, "%s:%s", address->ip, port_buf);
    return str;
}

char* tamaf_address_get_http_link(const tamaf_address_t* address) {
    if (!address) return NULL;
    char* addr_str = tamaf_address_get_string(address);
    if (!addr_str) return NULL;
    size_t len = 7 + strlen(addr_str) + 1;
    char* link = (char*)malloc(len);
    if (link) sprintf(link, "http://%s", addr_str);
    free(addr_str); return link;
}

char* tamaf_address_get_local_ip() {
    const char* default_ip = "127.0.0.1";
#ifdef _WIN32
    static bool wsa_started = false;
    if (!wsa_started) { WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData); wsa_started = true; }
#endif
    int sock = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return strdup(default_ip);
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");
    serv.sin_port = htons(53);
    if (connect(sock, (const struct sockaddr*)&serv, sizeof(serv)) < 0) { closesocket(sock); return strdup(default_ip); }
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr*)&name, &namelen) < 0) { closesocket(sock); return strdup(default_ip); }
    char* ip = strdup(inet_ntoa(name.sin_addr));
    closesocket(sock); return ip;
}
