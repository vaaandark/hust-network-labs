#ifndef SERVER_H_
#define SERVER_H_

#include "def.h"
#include <stdlib.h>

typedef struct {
    const char *suffix;
    size_t suffix_size;
    const char *mime_type;
} MimeType;

typedef struct {
    char ip[MAX_IP_LEN];
    char port[MAX_PORT_LEN];
    char root[MAX_ROOT_LEN];
} ServerInfo;

ServerInfo server_new(const char *config);

int server_setup(const ServerInfo *server_info);

void server_go(const ServerInfo *server_info, int listener);

#endif
