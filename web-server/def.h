#ifndef DEF_H_
#define DEF_H_

#define BACKLOG 10

#define MAX_BUFF_SIZE (1024 * 1024 * 10)
#define MAX_IP_LEN 64
#define MAX_PORT_LEN 8
#define MAX_URL_LEN 32
#define MAX_ROOT_LEN 32
#define MAX_PATH_LEN 128

#define INFO(...) \
    fprintf(stderr, "[FILE %s | LINE %d] ", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__)

#endif
