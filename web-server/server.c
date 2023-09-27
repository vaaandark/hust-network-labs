#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include "file.h"
#include "server.h"

static const MimeType *get_mime_type(const char *url, size_t url_len) {
    static const MimeType mime[] = {
        { "png", 3, "image/png" },
        { "jpeg", 4, "image/jpeg" },
        { "jpe", 3, "image/jpeg" },
        { "jpg", 3, "image/jpeg" },
        { "htm", 3, "text/html" },
        { "html", 4, "text/html" },
    };
    for (int i = url_len - 1; i > 0 && url[i] != '.'; --i) {
        const char *suffix = url + i;
        for (size_t j = 0; j < sizeof mime / sizeof(MimeType); ++j) {
            if (strcmp(suffix, mime[j].suffix) == 0) {
                return &mime[j];
            }
        }
    }
    return NULL;
}

ServerInfo server_new(const char *config) {
    FILE *fp = fopen(config, "r");
    ServerInfo server_info;
    char *ip = server_info.ip;
    char *port = server_info.port;
    char *root = server_info.root;
    if (fp == NULL) {
        fprintf(stderr, "cannot open configuration file: %s\n", config);
        exit(EXIT_FAILURE);
    }
    char buff[MAX_PATH_LEN];
    while (fgets(buff, sizeof buff - 1, fp) != NULL) {
        char *split= strchr(buff, ':');
        *split = '\0';
        char *val = split + 1;
        // strim
        while (isspace(*val)) {
            val++;
        }
        size_t len = strlen(val);
        if (len > 1 && val[len - 1] == '\n') {
            val[len - 1] = '\0';
        }
        if (strcmp(buff, "ip") == 0) {
            strcpy(ip, val);
        } else if (strcmp(buff, "port") == 0) {
            strcpy(port, val);
        } else if (strcmp(buff, "root") == 0) {
            strcpy(root, val);
        } else {
            fprintf(stderr, "wrong configuration file: %s\n", config);
            exit(EXIT_FAILURE);
        }
    }
    fclose(fp);
    INFO("listen at %s:%s, root directory is %s\n", ip, port, root);
    return server_info;
}

int server_setup(const ServerInfo *server_info) {
    const char *ip = server_info->ip;
    const char *port = server_info->port;
    struct addrinfo *available, *p;
    int yes = 1;
    static struct addrinfo hints;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(ip, port, &hints, &available) != 0 || available == NULL) {
        return -1;
    }
    int listener;
    for (p = available; p != NULL; p = p->ai_next) {
        if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
            close(listener);
            continue;
        }
        break;
    }
    freeaddrinfo(available);
    return listener;
}

typedef struct {
    int connection;
    const char *root;
} RequestInfo;

static void *handle_request(void *param) {
    int connection = ((RequestInfo *)param)->connection;
    const char *root = ((RequestInfo *)param)->root;
    static char req[MAX_BUFF_SIZE];
    ssize_t received = recv(connection, req, sizeof req, 0);
    if (received <= 0) {
        return NULL;
    }
    INFO("The HTTP message is as follows:\n%s", req);
    //    const char *method = req;
    char url[MAX_URL_LEN];
    const char *url_start = strchr(req, ' ') + 1;
    const char *url_end = strchr(url_start, ' ');
    size_t url_len = url_end - url_start;
    strncpy(url, url_start, url_end - url_start);
    url[url_len] = 0;
    INFO("Request URL is %s\n", url);
    const MimeType *mime = get_mime_type(url, url_len);
    if (mime != NULL) {
        INFO("Mime type is %s\n", mime->mime_type);
    }
    char filename[MAX_PATH_LEN];
    snprintf(filename, sizeof filename - 1, "%s%s", root, url);
    INFO("File name is %s\n", filename);
    LoadedFile *loaded_file = loaded_file_new(filename);
    static char response_buff[MAX_BUFF_SIZE];
    size_t response_len = 0;
    if (loaded_file == NULL) {
        INFO("404 Not Found\n");
        snprintf(response_buff, sizeof response_buff - 1,
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "404 Not Found");
        response_len = strlen(response_buff);
        if (send(connection, response_buff, response_len, 0) == -1) {
            INFO("Send error\n");
        }
        return NULL;
    }
    snprintf(response_buff, sizeof response_buff - 1,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "\r\n",
            mime->mime_type);
    response_len = strlen(response_buff);
    memcpy(response_buff + response_len, loaded_file->buff, loaded_file->content_size);
    response_len += loaded_file->content_size;
    loaded_file_drop(loaded_file);
    if (send(connection, response_buff, response_len, 0) == -1) {
        INFO("Send error\n");
    }
    close(connection);
    return NULL;
}

void server_go(const ServerInfo *server_info, int listener) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    if (listen(listener, BACKLOG) == -1) {
        INFO("Listen error\n");
    }
    for (;;) {
        int connection = accept(listener, (struct sockaddr *)&their_addr, &addr_size);
        if (connection == -1) {
            continue;
        }
        struct sockaddr_in client_addr;
        socklen_t client_client_addr = sizeof(client_addr);
        if (getpeername(connection, (struct sockaddr *)&client_addr, &client_client_addr) == 0) {
            char client_ip[INET_ADDRSTRLEN];
            int client_port = ntohs(client_addr.sin_port);
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
            INFO("New client from %s:%d\n", client_ip, client_port);
        }
        pthread_t thread_id;
        RequestInfo request_info = {
            connection, server_info->root
        };
        pthread_create(&thread_id, NULL, handle_request, (void *)&request_info);
    }
}
